/*
 * %kadu copyright begin%
 * Copyright 2009, 2010, 2011 Piotr Galiszewski (piotr.galiszewski@kadu.im)
 * Copyright 2009 Tomasz Rostański (rozteck@interia.pl)
 * Copyright 2008, 2009 Michał Podsiadlik (michal@kadu.net)
 * Copyright 2010, 2011, 2012, 2013, 2014 Bartosz Brachaczek (b.brachaczek@gmail.com)
 * Copyright 2008, 2009, 2010, 2011, 2013, 2014 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#include "unix-signal-handler.h"

#include "activate.h"
#include "configuration/configuration-api.h"
#include "configuration/configuration.h"
#include "core/application.h"
#include "core/crash-aware-object.h"
#include "core/version-service.h"
#include "exports.h"
#include "kadu-config.h"
#include "misc/paths-provider.h"
#include "plugin/activation/plugin-activation-service.h"
#include "windows/kadu-window-service.h"
#include "windows/kadu-window.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QDateTime>
#include <signal.h>
#include <stdio.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>

#if HAVE_EXECINFO
#include <execinfo.h>
#endif   // HAVE_EXECINFO

static void kadu_signal_handler(int signal);

Application *g_application{nullptr};
Configuration *g_configuration{nullptr};
KaduWindowService *g_kaduWindowService{nullptr};
PathsProvider *g_pathsProvider{nullptr};
PluginActivationService *g_pluginActivationService{nullptr};
VersionService *g_versionService{nullptr};

UnixSignalHandler::UnixSignalHandler(QObject *parent) : QObject{parent}
{
}

UnixSignalHandler::~UnixSignalHandler()
{
}

void UnixSignalHandler::setApplication(Application *application)
{
    g_application = application;
}

void UnixSignalHandler::setConfiguration(Configuration *configuration)
{
    g_configuration = configuration;
}

void UnixSignalHandler::setKaduWindowService(KaduWindowService *kaduWindowService)
{
    g_kaduWindowService = kaduWindowService;
}

void UnixSignalHandler::setPathsProvider(PathsProvider *pathsProvider)
{
    g_pathsProvider = pathsProvider;
}

void UnixSignalHandler::setPluginActivationService(PluginActivationService *pluginActivationService)
{
    g_pluginActivationService = pluginActivationService;
}

void UnixSignalHandler::setVersionService(VersionService *versionService)
{
    g_versionService = versionService;
}

void UnixSignalHandler::startSignalHandling()
{
    char *d = getenv("SIGNAL_HANDLING");
    bool signalHandlingEnabled = d ? (atoi(d) != 0) : true;

    if (signalHandlingEnabled)
    {
        signal(SIGSEGV, kadu_signal_handler);
        signal(SIGINT, kadu_signal_handler);
        signal(SIGTERM, kadu_signal_handler);
        signal(SIGUSR1, kadu_signal_handler);
        signal(SIGPIPE, SIG_IGN);
    }
}

static void kadu_signal_handler(int signal)
{
    static int sigsegvCount = 0;

    if (sigsegvCount > 1)
    {
        abort();
    }

    if (signal == SIGSEGV)
    {
        ++sigsegvCount;

        CrashAwareObject::notifyCrash();

        QString backtraceFileName =
            QStringLiteral("kadu.backtrace.") + QDateTime::currentDateTime().toString("yyyy.MM.dd.hh.mm.ss");

#if HAVE_EXECINFO
        void *backtraceArray[100];
        char **backtraceStrings;
        int numEntries;

        if ((numEntries = backtrace(backtraceArray, 100)) < 0)
            abort();
        if (!(backtraceStrings = backtrace_symbols(backtraceArray, numEntries)))
            abort();

        fprintf(stderr, "\n======= BEGIN OF BACKTRACE =====\n");
        for (int i = 0; i < numEntries; ++i)
            fprintf(stderr, "[%d] %s\n", i, backtraceStrings[i]);
        fprintf(stderr, "======= END OF BACKTRACE  ======\n");
        fflush(stderr);

        FILE *backtraceFile = fopen(qPrintable(QString(g_pathsProvider->profilePath() + backtraceFileName)), "w");
        if (backtraceFile)
        {
            fprintf(backtraceFile, "======= BEGIN OF BACKTRACE =====\n");
            for (int i = 0; i < numEntries; ++i)
                fprintf(backtraceFile, "[%d] %s\n", i, backtraceStrings[i]);
            fprintf(backtraceFile, "======= END OF BACKTRACE  ======\n");

            fprintf(backtraceFile, "loaded plugins:\n");
            auto pluginNames = g_pluginActivationService->activePlugins();
            for (auto const &pluginName : pluginNames)
                fprintf(backtraceFile, "> %s\n", qPrintable(pluginName));
            fprintf(backtraceFile, "Kadu version: %s\n", qPrintable(g_versionService->version()));
            fprintf(backtraceFile, "Qt compile time version: %s\nQt runtime version: %s\n", QT_VERSION_STR, qVersion());
#ifdef __GNUC__
            fprintf(backtraceFile, "GCC version: %d.%d.%d\n", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
#endif   // __GNUC__
            fprintf(backtraceFile, "EOF\n");

            fclose(backtraceFile);
        }

        free(backtraceStrings);
#endif   // HAVE_EXECINFO

        g_application->backupConfiguration();
        abort();
    }
    else if (signal == SIGUSR1)
    {
        _activateWindow(g_configuration, g_kaduWindowService->kaduWindow());
    }
    else if (signal == SIGINT || signal == SIGTERM)
        QCoreApplication::quit();
}
