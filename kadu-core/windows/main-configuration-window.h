#pragma once

#include "exports.h"
#include "os/generic/compositing-aware-object.h"
#include "windows/configuration-window.h"

#include <QtCore/QPointer>
#include <injeqt/injeqt.h>

class QCheckBox;
class QLineEdit;
class QSlider;

class AccountManager;
class BuddyListBackgroundColorsWidget;
class ConfigComboBox;
class ConfigLineEdit;
class ConfigurationUiHandlerRepository;
class IconsManager;
class IconThemeManager;
class KaduWindowService;
class LanguagesManager;
class PathsProvider;
class PluginListWidget;
class Preview;
class SyntaxEditorWindow;

class MainConfigurationWindow;

class ConfigurationUiHandler;

class KADUAPI MainConfigurationWindow : public ConfigurationWindow, CompositingAwareObject
{
    Q_OBJECT

    QPointer<AccountManager> m_accountManager;
    QPointer<ConfigurationUiHandlerRepository> m_configurationUiHandlerRepository;
    QPointer<IconsManager> m_iconsManager;
    QPointer<IconThemeManager> m_iconThemeManager;
    QPointer<KaduWindowService> m_kaduWindowService;
    QPointer<LanguagesManager> m_languagesManager;
    QPointer<PathsProvider> m_pathsProvider;

    QPointer<ConfigurationWindow> lookChatAdvanced;

    QCheckBox *onStartupSetLastDescription;
    QLineEdit *onStartupSetDescription;
    QCheckBox *userboxTransparency;
    QLineEdit *disconnectDescription;
    QSlider *userboxAlpha;
    QCheckBox *userboxBlur;
    BuddyListBackgroundColorsWidget *buddyColors;
    PluginListWidget *PluginList;

    void setLanguages();

    virtual void compositingEnabled();
    virtual void compositingDisabled();

private slots:
    INJEQT_SET void setAccountManager(AccountManager *accountManager);
    INJEQT_SET void
    setConfigurationUiHandlerRepository(ConfigurationUiHandlerRepository *configurationUiHandlerRepository);
    INJEQT_SET void setIconsManager(IconsManager *iconsManager);
    INJEQT_SET void setIconThemeManager(IconThemeManager *iconThemeManager);
    INJEQT_SET void setKaduWindowService(KaduWindowService *kaduWindowService);
    INJEQT_SET void setLanguagesManager(LanguagesManager *languagesManager);
    INJEQT_SET void setPathsProvider(PathsProvider *pathsProvider);
    INJEQT_INIT void init();

    void onChangeStartupStatus(int index);
    void onChangeStartupDescription(int index);
    void onChangeShutdownStatus(int index);
    void showLookChatAdvanced();
    void installIconTheme();
    void setIconThemes();
    void applied();
    void configurationUiHandlerAdded(ConfigurationUiHandler *configurationUiHandler);
    void configurationUiHandlerRemoved(ConfigurationUiHandler *configurationUiHandler);

public:
    static const char *SyntaxText;
    static const char *SyntaxTextNotify;

    explicit MainConfigurationWindow(ConfigurationWindowDataManager *dataManager, QWidget *parent);
    virtual ~MainConfigurationWindow();

    virtual void show();
};
