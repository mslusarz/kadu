/*
 * %kadu copyright begin%
 * Copyright 2008, 2009, 2010, 2010, 2011 Piotr Galiszewski (piotr.galiszewski@kadu.im)
 * Copyright 2010, 2012 Wojciech Treter (juzefwt@gmail.com)
 * Copyright 2008, 2010 Tomasz Rostański (rozteck@interia.pl)
 * Copyright 2011 Piotr Dąbrowski (ultr@ultr.pl)
 * Copyright 2009 Michał Podsiadlik (michal@kadu.net)
 * Copyright 2007, 2008, 2009, 2010, 2011, 2012, 2013 Rafał Malinowski (rafal.przemyslaw.malinowski@gmail.com)
 * Copyright 2010, 2011, 2012, 2013 Bartosz Brachaczek (b.brachaczek@gmail.com)
 * Copyright 2007 Dawid Stawiarski (neeo@kadu.net)
 * Copyright 2005, 2006, 2007 Marcin Ślusarz (joi@kadu.net)
 * %kadu copyright end%
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * Copyright for copying and drag'n'drop code from Psi+:
 *
 * Copyright (C) 2010 senu, Rion
 */

#include <QtCore/QEvent>
#include <QtCore/QFile>
#include <QtCore/QMimeData>
#include <QtCore/QPoint>
#include <QtCore/QString>
#include <QtCore/QTimer>
#include <QtCore/QUrl>
#include <QtCore/QWeakPointer>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QClipboard>
#include <QtGui/QContextMenuEvent>
#include <QtGui/QDrag>
#include <QtGui/QFileDialog>
#include <QtGui/QImage>
#include <QtGui/QMenu>
#include <QtGui/QMouseEvent>
#include <QtGui/QStyle>
#include <QtGui/QTextDocument>
#include <QtWebKit/QWebHistory>
#include <QtWebKit/QWebHitTestResult>
#include <QtWebKit/QWebPage>

#ifdef DEBUG_ENABLED
#	include <QtWebKit/QWebInspector>
#endif

#include "configuration/configuration-file.h"
#include "core/core.h"
#include "gui/services/clipboard-html-transformer-service.h"
#include "gui/windows/message-dialog.h"
#include "protocols/services/chat-image-service.h"
#include "services/image-storage-service.h"
#include "url-handlers/url-handler-manager.h"

#include "debug.h"

#include "kadu-web-view.h"

KaduWebView::KaduWebView(QWidget *parent) :
		QWebView(parent), DraggingPossible(false), IsLoading(false), RefreshTimer(new QTimer(this))
{
	kdebugf();

	QWebSettings::setMaximumPagesInCache(0);
	QWebSettings::setObjectCacheCapacities(0, 0, 0);

	setAttribute(Qt::WA_NoBackground);
	setAcceptDrops(false);
	setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing |
	               QPainter::SmoothPixmapTransform | QPainter::HighQualityAntialiasing);

	setPage(page());

	connect(RefreshTimer, SIGNAL(timeout()), this, SLOT(reload()));

	kdebugf2();
}

KaduWebView::~KaduWebView()
{
}

void KaduWebView::setImageStorageService(ImageStorageService *imageStorageService)
{
	CurrentImageStorageService = imageStorageService;
}

ImageStorageService * KaduWebView::imageStorageService() const
{
	return CurrentImageStorageService.data();
}

void KaduWebView::setPage(QWebPage *page)
{
	QWebView::setPage(page);
	page->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);

	page->history()->setMaximumItemCount(0);

	connect(page, SIGNAL(linkClicked(const QUrl &)), this, SLOT(hyperlinkClicked(const QUrl &)));
	connect(page, SIGNAL(loadStarted()), this, SLOT(loadStarted()));
	connect(page, SIGNAL(loadFinished(bool)), this, SLOT(loadFinishedSlot(bool)));
	connect(pageAction(QWebPage::Copy), SIGNAL(triggered()), this, SLOT(textCopied()));
	connect(pageAction(QWebPage::DownloadImageToDisk), SIGNAL(triggered()), this, SLOT(saveImage()));
}

void KaduWebView::contextMenuEvent(QContextMenuEvent *e)
{
	if (IsLoading)
		return;

	ContextMenuPos = e->pos();
	const QWebHitTestResult &hitTestContent = page()->currentFrame()->hitTestContent(ContextMenuPos);
	bool isImage = hitTestContent.imageUrl().isValid();
	bool isLink = hitTestContent.linkUrl().isValid();

	QAction *copy = pageAction(QWebPage::Copy);
	copy->setText(tr("Copy"));
	QAction *copyLink = pageAction(QWebPage::CopyLinkToClipboard);
	copyLink->setText(tr("Copy Link Address"));
	copyLink->setEnabled(isLink);
	QAction *copyImage = pageAction(QWebPage::CopyImageToClipboard);
	copyImage->setText(tr("Copy Image"));
	copyImage->setEnabled(isImage);
	QAction *saveImage = pageAction(QWebPage::DownloadImageToDisk);
	saveImage->setText(tr("Save Image"));
	saveImage->setEnabled(isImage);

	QMenu popupMenu(this);

	popupMenu.addAction(copy);
// 	popupmenu.addSeparator();
	popupMenu.addAction(copyLink);
// 	popupmenu.addAction(pageAction(QWebPage::DownloadLinkToDisk));
	popupMenu.addSeparator();
	popupMenu.addAction(copyImage);
	popupMenu.addAction(saveImage);

#ifdef DEBUG_ENABLED
	QAction *runInspector = new QAction(&popupMenu);
	runInspector->setText(tr("Run Inspector"));
	connect(runInspector, SIGNAL(triggered(bool)), this, SLOT(runInspector(bool)));

	popupMenu.addSeparator();
	popupMenu.addAction(runInspector);
#endif

 	popupMenu.exec(e->globalPos());
 	kdebugf2();
}

// taken from Psi+'s webkit patch, SVN rev. 2638, and slightly modified
void KaduWebView::mouseMoveEvent(QMouseEvent *e)
{
	if (!DraggingPossible || !(e->buttons() & Qt::LeftButton))
	{
		QWebView::mouseMoveEvent(e);
		return;
	}

	if ((e->pos() - DragStartPosition).manhattanLength() < QApplication::startDragDistance())
		return;

	QDrag *drag = new QDrag(this);
	QMimeData *mimeData = new QMimeData();

	QClipboard *clipboard = QApplication::clipboard();
	QMimeData *originalData = new QMimeData();
	foreach (const QString &format, clipboard->mimeData(QClipboard::Clipboard)->formats())
		originalData->setData(format, clipboard->mimeData(QClipboard::Clipboard)->data(format));
	// Do not use triggerPageAction(), see bug #2345.
	pageAction(QWebPage::Copy)->trigger();

	mimeData->setText(clipboard->mimeData()->text());
	mimeData->setHtml(clipboard->mimeData()->html());
	clipboard->setMimeData(originalData);
	drag->setMimeData(mimeData);

	drag->exec(Qt::CopyAction);
}

// taken from Psi+'s webkit patch, SVN rev. 2638, and slightly modified
void KaduWebView::mousePressEvent(QMouseEvent *e)
{
	if (IsLoading)
		return;

	QWebView::mousePressEvent(e);
	if ((e->buttons() & Qt::LeftButton) && page()->mainFrame()->hitTestContent(e->pos()).isContentSelected())
	{
		QSize cs = page()->mainFrame()->contentsSize();
		QSize vs = page()->viewportSize();
		QSize scrollBarsSize = QSize(cs.height() > vs.height() ? 1 : 0, cs.width() > vs.width() ? 1 : 0) *
				style()->pixelMetric(QStyle::PM_ScrollBarExtent);
		QRect visibleContentsRect = QRect(QPoint(0,0), vs - scrollBarsSize);
		DraggingPossible = visibleContentsRect.contains(e->pos());
		DragStartPosition = e->pos();
	}
	else
		DraggingPossible = false;
}

void KaduWebView::mouseReleaseEvent(QMouseEvent *e)
{
	kdebugf();
	QWebView::mouseReleaseEvent(e);
	DraggingPossible = false;

#ifdef Q_WS_X11
	if (!page()->selectedText().isEmpty())
		convertClipboardHtml(QClipboard::Selection);
#endif
}

void KaduWebView::hyperlinkClicked(const QUrl &anchor) const
{
	UrlHandlerManager::instance()->openUrl(anchor.toEncoded());
}

void KaduWebView::loadStarted()
{
	IsLoading = true;
}

void KaduWebView::loadFinishedSlot(bool success)
{
	Q_UNUSED(success)

	IsLoading = false;
}

void KaduWebView::refreshLater()
{
	RefreshTimer->setSingleShot(true);
	RefreshTimer->start(10);
}

void KaduWebView::saveImage()
{
	kdebugf();

	QUrl imageUrl = page()->currentFrame()->hitTestContent(ContextMenuPos).imageUrl();
	if (CurrentImageStorageService)
		imageUrl = CurrentImageStorageService.data()->toFileUrl(imageUrl);

	QString imageFullPath = imageUrl.toLocalFile();
	if (imageFullPath.isEmpty())
		return;

	QImage image;
	QString fileExt = '.' + imageFullPath.section('.', -1);
	bool formatUnknown = false;
	if (fileExt == "." || fileExt.contains('/'))
	{
		// TODO: we'd better guess the file format and not use QImage
		fileExt = ".png";
		formatUnknown = true;

		if (!image.load(imageFullPath))
		{
			MessageDialog::show(KaduIcon("dialog-warning"), tr("Kadu"), tr("Cannot save this image"));
			return;
		}
	}

	QWeakPointer<QFileDialog> fd = new QFileDialog(this);
	fd.data()->setFileMode(QFileDialog::AnyFile);
	fd.data()->setAcceptMode(QFileDialog::AcceptSave);
	fd.data()->setDirectory(config_file.readEntry("Chat", "LastImagePath"));
	fd.data()->setFilter(QString("%1 (*%2)").arg(qApp->translate("ImageDialog", "Images"), fileExt));
	fd.data()->setLabelText(QFileDialog::FileName, imageFullPath.section('/', -1));
	fd.data()->setWindowTitle(tr("Save image"));

	do
	{
		if (fd.data()->exec() != QFileDialog::Accepted)
			break;
		if (fd.data()->selectedFiles().isEmpty())
			break;

		QString file = fd.data()->selectedFiles().at(0);
		if (QFile::exists(file))
		{
			MessageDialog *dialog = MessageDialog::create(KaduIcon("dialog-question"), tr("Kadu"), tr("File already exists. Overwrite?"));
			dialog->addButton(QMessageBox::Yes, tr("Overwrite"));
			dialog->addButton(QMessageBox::No, tr("Cancel"));

			if (dialog->ask())
			{
				QFile removeMe(file);
				if (!removeMe.remove())
				{
					MessageDialog::show(KaduIcon("dialog-warning"), tr("Kadu"), tr("Cannot save image: %1").arg(removeMe.errorString()));
					continue;
				}
			}
			else
				continue;
		}

		QString dst = file;
		if (!dst.endsWith(fileExt))
			dst.append(fileExt);

		if (formatUnknown)
		{
			if (!image.save(dst, "PNG"))
			{
				MessageDialog::show(KaduIcon("dialog-warning"), tr("Kadu"), tr("Cannot save image"));
				continue;
			}
		}
		else
		{
			QFile src(imageFullPath);
			if (!src.copy(dst))
			{
				MessageDialog::show(KaduIcon("dialog-warning"), tr("Kadu"), tr("Cannot save image: %1").arg(src.errorString()));
				continue;
			}
		}

		config_file.writeEntry("Chat", "LastImagePath", fd.data()->directory().absolutePath());
	} while (false);

	delete fd.data();
}

#ifdef DEBUG_ENABLED
void KaduWebView::runInspector(bool toggled)
{
	Q_UNUSED(toggled)

	page()->settings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);

	QWebInspector *inspector = new QWebInspector();
	inspector->setPage(page());

	inspector->show();
}
#endif

void KaduWebView::textCopied() const
{
	convertClipboardHtml(QClipboard::Clipboard);
}

// taken from Psi+'s webkit patch, SVN rev. 2638, and slightly modified
void KaduWebView::convertClipboardHtml(QClipboard::Mode mode)
{
	QString html = QApplication::clipboard()->mimeData(mode)->html();

	if (Core::instance()->clipboardHtmlTransformerService())
		html = Core::instance()->clipboardHtmlTransformerService()->transform(html);

	QTextDocument document;
	document.setHtml(html);
	QMimeData *data = new QMimeData();
	data->setHtml(html);

	// remove OBJECT REPLACEMENT CHARACTER
	// see http://www.kadu.im/redmine/issues/2490
	data->setText(document.toPlainText().remove(QChar(0xfffc)));
	QApplication::clipboard()->setMimeData(data, mode);
}

void KaduWebView::setUserFont(const QString &fontString, bool force)
{
	QString style;

	if (fontString.isEmpty())
		style = "* { font-family: sans-serif; }";
	else
	{
		QFont font;
		font.fromString(fontString);
		style = QString("* { %1 }").arg(userFontStyle(font, force));
	}

	style.append("\
		img.scalable { max-width: 80%; }\
		img.scalable.unscaled { max-width: none; }\
	");

	QString url = QString("data:text/css;charset=utf-8;base64,%1").arg(QString(style.toUtf8().toBase64()));
	settings()->setUserStyleSheetUrl(url);
}

QString KaduWebView::userFontStyle(const QFont &font, bool force)
{
	QString style = "font-family:\"" + font.family() + "\",sans-serif" + (force ? " !important;" : ";");
	if (force && font.pointSize() != -1)
		style += QString(" font-size:%1pt;").arg(font.pointSize());
	return style;
}

#include "moc_kadu-web-view.cpp"
