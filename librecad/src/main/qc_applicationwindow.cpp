/****************************************************************************
**
** This file is part of the LibreCAD project, a 2D CAD program
** 
** Copyright (C) 2019 Shawn Curry (noneyabiz@mail.wasent.cz)
** Copyright (C) 2018 Simon Wells (simonrwells@gmail.com)
** Copyright (C) 2015-2016 ravas (github.com/r-a-v-a-s)
** Copyright (C) 2015 A. Stebich (librecad@mail.lordofbikes.de)
** Copyright (C) 2010 R. van Twisk (librecad@rvt.dds.nl)
** Copyright (C) 2001-2003 RibbonSoft. All rights reserved.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file gpl-2.0.txt included in the
** packaging of this file.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
**
** This copyright notice MUST APPEAR in all copies of the script!
**
**********************************************************************/

// Changes: https://github.com/LibreCAD/LibreCAD/commits/master/librecad/src/main/qc_applicationwindow.cpp

#include "qc_applicationwindow.h"

#include <algorithm>
#include <cmath>

#include <QActionGroup>
#include <QApplication>
#include <QByteArray>
#include <QAbstractButton>
#include <QDialogButtonBox>
#include <QDockWidget>
#include <QDir>
#include <QFileDialog>
#include <QFont>
#include <QFrame>
#include <QHash>
#include <QImageWriter>
#include <QInputDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QKeyEvent>
#include <QKeySequence>
#include <QLabel>
#include <QMdiArea>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPagedPaintDevice>
#include <QPluginLoader>
#include <QPixmap>
#include <QPrinter>
#include <QPrintDialog>
#include <QRadioButton>
#include <QRegExp>
#include <QSaveFile>
#include <QSet>
#include <QSignalBlocker>
#include <QShortcut>
#include <QSizePolicy>
#include <QSplitter>
#include <QStatusBar>
#include <QStyleFactory>
#include <QSysInfo>
#include <QTabBar>
#include <QTabWidget>
#include <QTimer>
#include <QToolBar>
#include <QToolButton>
#include <QVariant>
#include <QtSvg>

#include <boost/version.hpp>


#include "comboboxoption.h"
#include "doc_plugin_interface.h"
#include "main.h"
#include "textfileviewer.h"
#include "twostackedlabels.h"
#include "widgetcreator.h"
#include "kuubikcurrentlayerselector.h"
#include "kuubikiconregistry.h"
#include "kuubikpropertiespalette.h"
#include "kuubikribbon.h"
#include "kuubiktheme.h"

#include "rs_actionlibraryinsert.h"
#include "rs_actioninterface.h"
#include "rs_actionprintpreview.h"
#include "rs_circle.h"
#include "rs_commands.h"
#include "rs_debug.h"
#include "rs_dialogfactory.h"
#include "rs_document.h"
#include "rs_eventhandler.h"
#include "rs_graphic.h"
#include "rs_insert.h"
#include "rs_layer.h"
#include "rs_layerlist.h"
#include "rs_line.h"
#include "rs_overlayline.h"
#include "rs_painterqt.h"
#include "rs_point.h"
#include "rs_snapper.h"
#include "rs_pen.h"
#include "rs_polyline.h"
#include "rs_settings.h"
#include "rs_staticgraphicview.h"
#include "rs_system.h"
#include "rs_selection.h"
#include "rs_units.h"

#include "lc_actionfactory.h"
#include "lc_actiongroupmanager.h"
#include "lc_actionmodifyduplicate.h"
#include "lc_centralwidget.h"
#include "lc_penwizard.h"
#include "lc_printing.h"
#include "lc_widgetfactory.h"
#include "lc_widgetoptionsdialog.h"
#include "lc_undosection.h"

#include "qc_dialogfactory.h"
#include "qc_mdiwindow.h"
#include "qc_plugininterface.h"

#include "qg_actionhandler.h"
#include "qg_activelayername.h"
#include "qg_blockwidget.h"
#include "qg_commandwidget.h"
#include "qg_coordinatewidget.h"
#include "qg_dimlinearoptions.h"
#include "qg_dimoptions.h"
#include "qg_dlgimageoptions.h"
#include "qg_exitdialog.h"
#include "qg_filedialog.h"
#include "qg_graphicview.h"
#include "qg_layerwidget.h"
#include "qg_lineoptions.h"
#include "lc_layertreewidget.h"
#include "qg_pentoolbar.h"
#include "qg_selectionwidget.h"
#include "qg_snaptoolbar.h"
#include "qg_mousewidget.h"
#include "qg_recentfiles.h"

#ifndef QC_APP_ICON
# define QC_APP_ICON ":/main/kuubikdraw.png"
#endif
#ifndef QC_ABOUT_ICON
# define QC_ABOUT_ICON ":/main/intro_kuubikdraw.png"
#endif

/*	- Window Title Bar Extra (character) Size.
 *	- Notes: Extra characters appearing in the windows title bar
 *	  are " - [", ... "]" (5), and sometimes "Print preview of " (17).
 *	*/
#define WTB_EXTRA_SIZE        (5 + 17)

/*	Window Title Bar Maximum Size.
 *	Notes: On Windows XP, this is 79.
 *	*/
#define WTB_MAX_SIZE        79

namespace {
void printMargins(const QMarginsF &margins, QString name)
{
    LC_ERR << name << " margins(mm): " << margins.left() << ": " << margins.top() << " : "
           << margins.right() << " : " << margins.bottom();
}

// show pen wizard by user settings
// Workaround for issue #2214: right dock area size may be over the screen size
bool usePenWizard()
{
    auto guard= RS_SETTINGS->beginGroupGuard("/CustomToolbars");
    return RS_SETTINGS->readNumEntry("/UsePenWizard", 0) == 1;
}

template <typename Contains, typename SetValue>
void applyPhaseThreeVisibilityDefaults(Contains contains, SetValue setValue)
{
    for (const QString& key : {
             QStringLiteral("KuubikStatus/Visible/ObjectSnap"),
             QStringLiteral("KuubikStatus/Visible/SnapTracking")}) {
        if (!contains(key)) {
            setValue(key, true);
        }
    }
}
}

/**
 * Constructor. Initializes the app.
 */
QC_ApplicationWindow::QC_ApplicationWindow()
    : ag_manager(new LC_ActionGroupManager(this))
    , actionHandler(new QG_ActionHandler(this))
    , current_subwindow(nullptr)
    , pen_wiz(new LC_PenWizard(QObject::tr("Pen Wizard"), this))
{
    RS_DEBUG->print("QC_ApplicationWindow::QC_ApplicationWindow");

#ifdef _WINDOWS
	qt_ntfs_permission_lookup++; // turn checking on
#endif

    //accept drop events to open files
    setAcceptDrops(true);

    // make the left and right dock areas dominant
    setCorner(Qt::TopLeftCorner, Qt::LeftDockWidgetArea);
    setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
    setCorner(Qt::TopRightCorner, Qt::RightDockWidgetArea);
    setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);

    QSettings settings;

    RS_DEBUG->print("QC_ApplicationWindow::QC_ApplicationWindow: setting icon");
    setWindowIcon(QIcon(QC_APP_ICON));

    pen_wiz->setObjectName("pen_wiz");
    connect(this, &QC_ApplicationWindow::windowsChanged,
            pen_wiz, &LC_PenWizard::setEnabled);
    addDockWidget(Qt::RightDockWidgetArea, pen_wiz);

    RS_DEBUG->print("QC_ApplicationWindow::QC_ApplicationWindow: init status bar");
    QStatusBar* status_bar = statusBar();
    coordinateWidget = new QG_CoordinateWidget(status_bar, "coordinates");
    status_bar->addWidget(coordinateWidget);
    mouseWidget = new QG_MouseWidget(status_bar, "mouse info");
    status_bar->addWidget(mouseWidget);
    selectionWidget = new QG_SelectionWidget(status_bar, "selections");
    status_bar->addWidget(selectionWidget);
    m_pActiveLayerName = new QG_ActiveLayerName(this);
    status_bar->addWidget(m_pActiveLayerName);
    grid_status = new TwoStackedLabels(status_bar);
    grid_status->setTopLabel(tr("Grid Status"));
    status_bar->addWidget(grid_status);

    settings.beginGroup("Widgets");
    int allow_statusbar_fontsize = settings.value("AllowStatusbarFontSize", 0).toInt();
    int allow_statusbar_height = settings.value("AllowStatusbarHeight", 0).toInt();

    if (allow_statusbar_fontsize)
    {
        int fontsize = settings.value("StatusbarFontSize", 12).toInt();
        QFont font;
        font.setPointSize(fontsize);
        status_bar->setFont(font);
    }
    int height {64};
    if (allow_statusbar_height) {
        height = settings.value( "StatusbarHeight", 64).toInt();
    }
    status_bar->setMinimumHeight( height);
    status_bar->setMaximumHeight( height);
    settings.endGroup();

    RS_DEBUG->print("QC_ApplicationWindow::QC_ApplicationWindow: creating LC_CentralWidget");

    auto central = new LC_CentralWidget(this);

    setCentralWidget(central);

    mdiAreaCAD = central->getMdiArea();
    mdiAreaCAD->setDocumentMode(true);

	RS_SETTINGS->beginGroup("/WindowOptions");
	setTabLayout(static_cast<RS2::TabShape>(RS_SETTINGS->readNumEntry("/TabShape", RS2::Triangular)),
		static_cast<RS2::TabPosition>(RS_SETTINGS->readNumEntry("/TabPosition", RS2::West)));
	RS_SETTINGS->endGroup();

    settings.beginGroup("Startup");
	if (settings.value("TabMode", 0).toBool()) {
		mdiAreaCAD->setViewMode(QMdiArea::TabbedView);
		QList<QTabBar *> tabBarList = mdiAreaCAD->findChildren<QTabBar*>();
		QTabBar *tabBar = tabBarList.at(0);
		if (tabBar)
			tabBar->setExpanding(false);
	}
        
    bool enable_left_sidebar = settings.value("EnableLeftSidebar", 1).toBool();
    bool enable_cad_toolbars = settings.value("EnableCADToolbars", 1).toBool();
    settings.endGroup();

    connect(mdiAreaCAD, SIGNAL(subWindowActivated(QMdiSubWindow*)),
            this, SLOT(slotWindowActivated(QMdiSubWindow*)));

    settings.beginGroup("Widgets");
    bool custom_size = settings.value("AllowToolbarIconSize", 0).toBool();
    int icon_size = custom_size ? settings.value("ToolbarIconSize", 24).toInt() : 24;
    settings.endGroup();

    if (custom_size)
        setIconSize(QSize(icon_size, icon_size));

    LC_ActionFactory a_factory(this, actionHandler);
    a_factory.using_theme = settings.value("Widgets/AllowTheme", 0).toBool();
    a_factory.fillActionContainer(a_map, ag_manager);

    LC_WidgetFactory widget_factory(this, a_map, ag_manager);
    if (enable_left_sidebar){
        int leftSidebarColumnsCount = settings.value("Widgets/LeftToolbarColumnsCount", 5).toInt();
        widget_factory.createLeftSidebar(leftSidebarColumnsCount, icon_size);
    }
    if (enable_cad_toolbars)
        widget_factory.createCADToolbars();
    widget_factory.createRightSidebar(actionHandler);
    widget_factory.createCategoriesToolbar();
    widget_factory.createStandardToolbars(actionHandler);

    foreach(auto action, widget_factory.snap_toolbar->actions())
    {
        if(!action->objectName().isEmpty())
        {
            a_map[action->objectName()] = action;
        }
    }

    settings.beginGroup("CustomToolbars");
    foreach (auto key, settings.childKeys())
    {
        auto toolbar = new QToolBar(key, this);
        toolbar->setObjectName(key);
        foreach (auto action, settings.value(key).toStringList())
        {
            toolbar->addAction(a_map[action]);
        }
        addToolBar(toolbar);
    }
    settings.endGroup();

    if (settings.value("Startup/FirstLoad", 1).toBool())
    {
        QStringList list;
        list << "DrawMText"
             << "DrawHatch"
             << "DrawImage"
             << "BlocksCreate"
             << "DrawPoint";

        auto toolbar = new QToolBar("DefaultCustom", this);
        toolbar->setObjectName("DefaultCustom");
        foreach (auto& action, list)
        {
            toolbar->addAction(a_map[action]);
        }
        settings.setValue("CustomToolbars/DefaultCustom", list);
        addToolBar(Qt::LeftToolBarArea, toolbar);
    }

    widget_factory.createMenus(menuBar());

    undoButton = a_map["EditUndo"];
    redoButton = a_map["EditRedo"];
    previousZoom = a_map["ZoomPrevious"];

    dock_areas.left = a_map["LeftDockAreaToggle"];
    dock_areas.right = a_map["RightDockAreaToggle"];
    dock_areas.top = a_map["TopDockAreaToggle"];
    dock_areas.bottom = a_map["BottomDockAreaToggle"];
    dock_areas.floating = a_map["FloatingDockwidgetsToggle"];

    snapToolBar = widget_factory.snap_toolbar;
    penToolBar = widget_factory.pen_toolbar;
    optionWidget = widget_factory.options_toolbar;

    layerWidget = widget_factory.layer_widget;

    layerTreeWidget = widget_factory.layer_tree_widget;
    /**/
    blockWidget = widget_factory.block_widget;
    commandWidget = widget_factory.command_widget;

    penPaletteWidget = widget_factory.pen_palette;

    file_menu = widget_factory.file_menu;
    windowsMenu = widget_factory.windows_menu;

    // Kuubik Ribbon owns presentation only. Every button keeps the original
    // LibreCAD QAction and therefore the upstream command implementation,
    // shortcut, enabled state and checked state.
    removeToolBar(penToolBar);
    removeToolBar(optionWidget);
    kuubikRibbon = new KuubikRibbon(a_map, penToolBar, optionWidget, this);
    kuubikRibbonToolbar = new QToolBar(tr("Kuubik Ribbon"), this);
    kuubikRibbonToolbar->setObjectName("kuubikRibbonToolbar");
    kuubikRibbonToolbar->setMovable(false);
    kuubikRibbonToolbar->setFloatable(false);
    kuubikRibbonToolbar->setAllowedAreas(Qt::TopToolBarArea);
    QSizePolicy ribbonToolbarPolicy = kuubikRibbonToolbar->sizePolicy();
    ribbonToolbarPolicy.setHorizontalPolicy(QSizePolicy::Ignored);
    kuubikRibbonToolbar->setSizePolicy(ribbonToolbarPolicy);
    kuubikRibbonToolbar->addWidget(kuubikRibbon);
    addToolBar(Qt::TopToolBarArea, kuubikRibbonToolbar);

    kuubikCurrentLayerSelector = new KuubikCurrentLayerSelector(this);
    kuubikRibbon->setCurrentLayerSelector(kuubikCurrentLayerSelector);
    connect(kuubikCurrentLayerSelector, &KuubikCurrentLayerSelector::layerStateChanged,
            this, [this]() {
        QC_MDIWindow* mdi = getMDIWindow();
        RS_Document* document = mdi == nullptr ? nullptr : mdi->getDocument();
        if (document != nullptr) {
            refreshKuubikProperties(document->countSelected(),
                                    document->totalSelectedLength());
        }
    });

    kuubikPropertiesDock = new QDockWidget(tr("Properties"), this);
    kuubikPropertiesDock->setObjectName(QStringLiteral("kuubikPropertiesDock"));
    kuubikPropertiesDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    kuubikPropertiesDock->setMinimumWidth(280);
    kuubikPropertiesPalette = new KuubikPropertiesPalette(kuubikPropertiesDock);
    kuubikPropertiesPalette->setModifyEntityAction(a_map.value("ModifyEntity", nullptr));
    kuubikPropertiesDock->setWidget(kuubikPropertiesPalette);
    addDockWidget(Qt::RightDockWidgetArea, kuubikPropertiesDock);
    if (auto* dockMenu = menuBar()->findChild<QMenu*>("dockwidgets_menu")) {
        dockMenu->addAction(kuubikPropertiesDock->toggleViewAction());
    }

    if (auto* toolbarsMenu = menuBar()->findChild<QMenu*>("toolbars_menu")) {
        toolbarsMenu->addAction(kuubikRibbonToolbar->toggleViewAction());
    }
    createKuubikWorkspaceMenu();
    createKuubikStatusControls();

    connect(a_map["FileClose"], SIGNAL(triggered(bool)),
            mdiAreaCAD, SLOT(closeActiveSubWindow()));

    connect(penToolBar, SIGNAL(penChanged(RS_Pen)),
            this, SLOT(slotPenChanged(const RS_Pen&)));

    auto ctrl_l = new QShortcut(QKeySequence("Ctrl+L"), this);
    connect(ctrl_l, SIGNAL(activated()), actionHandler, SLOT(slotLayersAdd()));

    auto ctrl_m = new QShortcut(QKeySequence("Ctrl+M"), this);
    connect(ctrl_m, SIGNAL(activated()), this, SLOT(slotFocusCommandLine()));

    // This event filter allows sending key events to the command widget, therefore, no
    // need to activate the command widget before typing commands.
    // Since this nice feature causes a bug of lost key events when the command widget is on
    // a screen different from the main window, disabled for the time being
    // send key events for mdiAreaCAD to command widget by default
    mdiAreaCAD->installEventFilter(commandWidget);

    RS_DEBUG->print("QC_ApplicationWindow::QC_ApplicationWindow: creating dialogFactory");
    dialogFactory = new QC_DialogFactory(this, optionWidget);
    RS_DEBUG->print("QC_ApplicationWindow::QC_ApplicationWindow: creating dialogFactory: OK");

    RS_DEBUG->print("setting dialog factory object");
    if (RS_DialogFactory::instance()==nullptr) {
        RS_DEBUG->print("no RS_DialogFactory instance");
    } else {
        RS_DEBUG->print("got RS_DialogFactory instance");
    }
    RS_DialogFactory::instance()->setFactoryObject(dialogFactory);
    RS_DEBUG->print("setting dialog factory object: OK");

    recentFiles = new QG_RecentFiles(this, 9);
    auto recent_menu = new QMenu(tr("Recent Files"), file_menu);
    file_menu->addMenu(recent_menu);
    recentFiles->addFiles(recent_menu);

    RS_DEBUG->print("QC_ApplicationWindow::QC_ApplicationWindow: init settings");
    initSettings();
    initializeKuubikVisuals();

    auto command_file = settings.value("Paths/VariableFile", "").toString();
    if (!command_file.isEmpty())
        commandWidget->leCommand->readCommandFile(command_file);

    // Activate autosave timer
    bool allowAutoSave = settings.value("Defaults/AutoBackupDocument", 1).toBool();
    startAutoSave(allowAutoSave);

    // Disable menu and toolbar items
    //emit windowsChanged(false);

    RS_COMMANDS->updateAlias();
    //plugin load
    loadPlugins();

    statusBar()->showMessage(qApp->applicationName() + " Ready", 2000);
}

void QC_ApplicationWindow::startAutoSave(bool startAutoBackup)
{
    if (startAutoBackup)
    {
        if (m_autosaveTimer == nullptr) {
            m_autosaveTimer = std::make_unique<QTimer>(this);
            m_autosaveTimer->setObjectName("autosave");
            connect(m_autosaveTimer.get(), SIGNAL(timeout()), this, SLOT(slotFileAutoSave()));
        }
        if (!m_autosaveTimer->isActive()) {
            // autosaving has been turned on. Make a backup immediately
            auto groupGuard = RS_SETTINGS->beginGroupGuard("/Defaults");
            RS_SETTINGS->writeEntry("/AutoBackupDocument", 1);
            slotFileAutoSave();
            int ms = 60000 * RS_SETTINGS->readNumEntry("AutoSaveTime", 5);
            m_autosaveTimer->start(ms);
        }
    } else {
        m_autosaveTimer.reset();
    }
}

/**
 * @brief QC_ApplicationWindow::getAppWindow() accessor for the application window singleton instance
 * @return QC_ApplicationWindow* the application window instance
 */
std::unique_ptr<QC_ApplicationWindow>& QC_ApplicationWindow::getAppWindow()
{
    static auto instance = std::unique_ptr<QC_ApplicationWindow>(new QC_ApplicationWindow);
    // singleton could be reset: cannot be called after reseting
    Q_ASSERT(instance != nullptr);
    return instance;
}

/**
  * Find a menu entry in the current menu list. This function will try to recursively find the menu
  * searchMenu for example foo/bar
  * thisMenuList list of Widgets
  * currentEntry only used internally during recursion
  * returns 0 when no menu was found
  */
QMenu *QC_ApplicationWindow::findMenu(const QString &searchMenu, const QObjectList thisMenuList, const QString& currentEntry) {
    if (searchMenu==currentEntry)
        return ( QMenu *)thisMenuList.at(0)->parent();

    QList<QObject*>::const_iterator i=thisMenuList.begin();
    while (i != thisMenuList.end()) {
        if ((*i)->inherits ("QMenu")) {
            QMenu *ii=(QMenu*)*i;
            if (QMenu *foundMenu=findMenu(searchMenu, ii->children(), currentEntry+"/"+ii->objectName().replace("&", ""))) {
                return foundMenu;
            }
        }
        ++i;
    }
    return 0;
}

/**
 * Arrange the sub-windows as specified, and set the setting.
 * Note: Tab mode always uses (and sets) the RS2::Maximized mode.
 * @param m the layout mode; if set to RS2::CurrentMode, read the current setting
 * @param actuallyDont just set the setting, don't actually do the arrangement
 */
void QC_ApplicationWindow::doArrangeWindows(RS2::SubWindowMode m, bool actuallyDont)
{
	RS_SETTINGS->beginGroup("/WindowOptions");
	int mode = m != RS2::CurrentMode ? m : RS_SETTINGS->readNumEntry("/SubWindowMode", RS2::Maximized);
	RS_SETTINGS->endGroup();
	if (!actuallyDont) switch (mode) {
	case RS2::Maximized:
		if (mdiAreaCAD->currentSubWindow())
			mdiAreaCAD->currentSubWindow()->showMaximized();
		break;
	case RS2::Cascade:
		slotCascade();
		break;
	case RS2::Tile:
		slotTile();
		break;
	case RS2::TileHorizontal:
		slotTileHorizontal();
		break;
	case RS2::TileVertical:
		slotTileVertical();
		break;
	}
	
	RS_SETTINGS->beginGroup("/WindowOptions");
	RS_SETTINGS->writeEntry("/SubWindowMode", mode);
	RS_SETTINGS->endGroup();
}

/**
 * Set the QTabWidget shape and position for the MDI area; also the settings.
 * Note: setting a Tab layout always sets the window arrangement to RS2::Maximized
 * Used by the Drawing > Layout menu.
 * @param s the tab shape; if RS2::AnyShape read the current setting
 * @param p the tab bar position; if RS2::AnyPosition read the current setting
 */
void QC_ApplicationWindow::setTabLayout(RS2::TabShape s, RS2::TabPosition p)
{
	RS_SETTINGS->beginGroup("/WindowOptions");
	int shape = s != RS2::AnyShape ? s : RS_SETTINGS->readNumEntry("/TabShape", RS2::Triangular);
	int position = p != RS2::AnyPosition ? p : RS_SETTINGS->readNumEntry("/TabPosition", RS2::West);
	RS_SETTINGS->endGroup();
	mdiAreaCAD->setTabShape(static_cast<QTabWidget::TabShape>(shape));
	mdiAreaCAD->setTabPosition(static_cast<QTabWidget::TabPosition>(position));
	doArrangeWindows(RS2::Maximized);
	RS_SETTINGS->beginGroup("/WindowOptions");
	RS_SETTINGS->writeEntry("/TabShape", shape);
	RS_SETTINGS->writeEntry("/TabPosition", position);
	RS_SETTINGS->endGroup();
}

/**
 * Force-Save(as) the content of the sub window.  Retry on failure.
 * @return true success (or window was not modified)
 * @return false user cancelled (or window was null)
 */
bool QC_ApplicationWindow::doSave(QC_MDIWindow * w, bool forceSaveAs)
{
	QString name, msg;
	bool cancelled;
	if (!w) return false;
    if (w->getDocument()->isModified() || forceSaveAs) {
		name = w->getDocument()->getFilename();
		if (name.isEmpty())
			doActivate(w); // show the user the drawing for save as
		msg = name.isEmpty() ? tr("Saving drawing...") : tr("Saving drawing: %1").arg(name);
		statusBar()->showMessage(msg);
		bool res = forceSaveAs ? w->slotFileSaveAs(cancelled) : w->slotFileSave(cancelled);
		if (res) {
			if (cancelled) {
				statusBar()->showMessage(tr("Save cancelled"), 2000);
				return false;
			}
			name = w->getDocument()->getFilename();			
			msg = tr("Saved drawing: %1").arg(name);
			statusBar()->showMessage(msg, 2000);
			commandWidget->appendHistory(msg);

            if (!recentFiles->contains(name)) {
                recentFiles->add(name);
            }

			w->setWindowTitle(format_filename_caption(name) + "[*]");
			if (w->getGraphicView()->isDraftMode())
				w->setWindowTitle(w->windowTitle() + " [" + tr("Draft Mode") + "]");

            auto groupGuard = RS_SETTINGS->beginGroupGuard("/Defaults");
            bool autoBackup = RS_SETTINGS->readNumEntry("/AutoBackupDocument", 1) == 1;
            startAutoSave(autoBackup);
		}
		else {
			msg = tr("Cannot save the file ") +
				w->getDocument()->getFilename()
				+ tr(" , please check the filename and permissions.");
			statusBar()->showMessage(msg, 2000);
			commandWidget->appendHistory(msg);
			return doSave(w, true);
		}
	}
	return true;
}

/**
 * Force-Close this sub window.
 * @param activateNext also activate the next window in the window_list, if any
 */
void QC_ApplicationWindow::doClose(QC_MDIWindow * w, bool activateNext)
{
	RS_DEBUG->print("QC_ApplicationWindow::doClose begin");
	w->getGraphicView()->killAllActions();
	if (!activedMdiSubWindow || activedMdiSubWindow == w) {
        if (kuubikCurrentLayerSelector != nullptr) {
            kuubikCurrentLayerSelector->setLayerList(nullptr);
        }
        if (kuubikPropertiesPalette != nullptr) {
            kuubikPropertiesPalette->setDocument(nullptr);
        }
    }
	QC_MDIWindow* parentWindow = w->getParentWindow();
	if (parentWindow)
	{
		RS_DEBUG->print("QC_ApplicationWindow::doClose closing block or print preview");
		parentWindow->removeChildWindow(w);
	}
	else
	{
		RS_DEBUG->print("QC_ApplicationWindow::doClose closing graphic");
	}
    foreach (auto&& child, w->getChildWindows()) // block editors and print previews; just force these closed
		doClose(child, false); // they belong to the document (changes already saved there)
	w->getChildWindows().clear();
	mdiAreaCAD->removeSubWindow(w);
	window_list.removeOne(w);

	if (!activedMdiSubWindow || activedMdiSubWindow == w)
	{
		layerWidget->setLayerList(nullptr, false);

        if (layerTreeWidget != nullptr) {
            layerTreeWidget->setLayerList(nullptr);
            layerTreeWidget->set_view(nullptr);
            layerTreeWidget->set_document(nullptr);
        }


		blockWidget->setBlockList(nullptr);
		coordinateWidget->setGraphic(nullptr);
	}

 if (penPaletteWidget != nullptr){
     penPaletteWidget->setLayerList(nullptr);
 }

	openedFiles.removeAll(w->getDocument()->getFilename());

	activedMdiSubWindow = nullptr;
	actionHandler->set_view(nullptr);
	actionHandler->set_document(nullptr);

    if (activateNext && !window_list.empty()) {
        if (nullptr != parentWindow) {
            doActivate(parentWindow);
        }
        else {
            doActivate(window_list.back());
        }
    }

    RS_DEBUG->print("QC_ApplicationWindow::doClose end");
}

/**
 * Force-Activate this sub window.
 */
void QC_ApplicationWindow::doActivate(QMdiSubWindow * w)
{
	RS_SETTINGS->beginGroup("/WindowOptions");
	bool maximized = RS_SETTINGS->readNumEntry("/Maximized");
	RS_SETTINGS->endGroup();
	if (w) {
		slotWindowActivated(w, true);
		w->activateWindow();
		w->raise();
		w->setFocus();
        if (maximized || QMdiArea::TabbedView == mdiAreaCAD->viewMode()) {
            w->showMaximized();
        }
        else {
            w->show();
        }
    }
	if (mdiAreaCAD->viewMode() == QMdiArea::SubWindowView)
		doArrangeWindows(RS2::CurrentMode);
	enableFileActions(qobject_cast<QC_MDIWindow*>(w));
}

/**
 * Show a Save/Close/Cancel(All) dialog for the content of this sub-window.
 * The window handle must not be null, and the document must actually have been modified.
 *
 * @param showSaveAll show a Save All button and rename Close -> Close All
 * @return QG_ExitDialog::ExitDialogResult the button that was pressed, or -1 if invoked in error
 * @see QG_ExitDialog
 */
int QC_ApplicationWindow::showCloseDialog(QC_MDIWindow * w, bool showSaveAll)
{
    LC_LOG<<"QC_ApplicationWindow::showCloseDialog(): begin";
	QG_ExitDialog dlg(this);
	dlg.setShowSaveAll(showSaveAll);
	dlg.setTitle(tr("Closing Drawing"));
	if (w && w->getDocument()->isModified()) {
		QString fn = w->getDocument()->getFilename();
		if (fn.isEmpty())
			fn = w->windowTitle();
		else if (fn.length() > 50)
			fn = QString("%1...%2").arg(fn.left(24)).arg(fn.right(24));

		dlg.setText(tr("Save changes to the following item?\n%1").arg(fn));
        LC_LOG<<"QC_ApplicationWindow::showCloseDialog(): showing dialog";
        return dlg.exec();
	}
    LC_LOG<<"QC_ApplicationWindow::showCloseDialog(): failed";
    return -1; // should never get here; please send only modified documents
}

/**
 * Enable the available file actions for this sub-window.
 */
void QC_ApplicationWindow::enableFileActions(QC_MDIWindow* w)
{
	if (!w || w->getDocument()->getFilename().isEmpty()) {
		a_map["FileSave"]->setText(tr("&Save"));
		a_map["FileSaveAs"]->setText(tr("Save &as..."));
	}
	else {
		QString name = format_filename_caption(w->getDocument()->getFilename());
		a_map["FileSave"]->setText(tr("&Save %1").arg(name));
		a_map["FileSaveAs"]->setText(tr("Save %1 &as...").arg(name));
	}
	a_map["FileSave"]->setEnabled(w);
	a_map["FileSaveAs"]->setEnabled(w);
	a_map["FileSaveAll"]->setEnabled(w && window_list.count() > 1);
	a_map["FileExportMakerCam"]->setEnabled(w);
	a_map["FilePrintPDF"]->setEnabled(w);
	a_map["FileExport"]->setEnabled(w);
	a_map["FilePrint"]->setEnabled(w);
	a_map["FilePrintPreview"]->setEnabled(w);
	a_map["FileClose"]->setEnabled(w);
	a_map["FileCloseAll"]->setEnabled(w && window_list.count() > 1);
}

/**
 * Loads the found plugins.
 */
void QC_ApplicationWindow::loadPlugins() {

    loadedPlugins.clear();
    QStringList lst = RS_SYSTEM->getDirectoryList("plugins");
    // Keep track of plugin filenames loaded to skip duplicate plugins.
    QStringList loadedPluginFileNames;

    for (int i = 0; i < lst.size(); ++i) {
        QDir pluginsDir(lst.at(i));
        for(const QString& fileName: pluginsDir.entryList(QDir::Files)) {
            // Skip loading a plugin if a plugin with the same
            // filename has already been loaded.
            #ifdef Q_OS_MAC
            if (!fileName.contains(".dylib"))
                continue;
            #endif
#if (defined (_WIN32) || defined (_WIN32) || defined (_WIN64))
            if (!fileName.contains(".dll"))
                continue;
#endif

            if (loadedPluginFileNames.contains(fileName)) {
                continue;
            }
            QPluginLoader pluginLoader(pluginsDir.absoluteFilePath(fileName));
            QObject *plugin = pluginLoader.instance();
            if (plugin) {
                QC_PluginInterface *pluginInterface = qobject_cast<QC_PluginInterface *>(plugin);
                if (pluginInterface) {
                    loadedPlugins.push_back(pluginInterface);
                    loadedPluginFileNames.push_back(fileName);
                    PluginCapabilities pluginCapabilities=pluginInterface->getCapabilities();
                    for(const PluginMenuLocation& loc: pluginCapabilities.menuEntryPoints) {
                        QAction *actpl = new QAction(loc.menuEntryActionName, plugin);
                        actpl->setData(loc.menuEntryActionName);
                        connect(actpl, SIGNAL(triggered()), this, SLOT(execPlug()));
                        connect(this, SIGNAL(windowsChanged(bool)), actpl, SLOT(setEnabled(bool)));
                        QMenu *atMenu = findMenu("/"+loc.menuEntryPoint, menuBar()->children(), "");
                        if (atMenu) {
                            atMenu->addAction(actpl);
                        } else {
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
                            QStringList treemenu = loc.menuEntryPoint.split('/', Qt::SkipEmptyParts);
#else
                            QStringList treemenu = loc.menuEntryPoint.split('/', QString::SkipEmptyParts);
#endif
                            QString currentLevel="";
                            QMenu *parentMenu=0;
                            do {
                                QString menuName=treemenu.at(0); treemenu.removeFirst();
                                currentLevel=currentLevel+"/"+menuName;
                                atMenu = findMenu(currentLevel, menuBar()->children(), "");
                                if (atMenu==0) {
                                    if (parentMenu==0) {
                                        parentMenu=menuBar()->addMenu(menuName);
                                    } else {
                                        parentMenu=parentMenu->addMenu(menuName);
                                    }
                                    parentMenu->setObjectName(menuName);
                                }
                            } while(treemenu.size()>0);
							if (parentMenu) parentMenu->addAction(actpl);
                        }
                    }
                }
            } else {
                QMessageBox::information(this, "Info", pluginLoader.errorString());
                RS_DEBUG->print("QC_ApplicationWindow::loadPlugin: %s", pluginLoader.errorString().toLatin1().data());
            }
        }
    }
}

/**
 * Execute the plugin.
 */
void QC_ApplicationWindow::execPlug() {
    QAction *action = qobject_cast<QAction *>(sender());
    QC_PluginInterface *plugin = qobject_cast<QC_PluginInterface *>(action->parent());
//get actual drawing
    QC_MDIWindow* w = getMDIWindow();
    RS_Document* currdoc = w->getDocument();
//create document interface instance
    Doc_plugin_interface pligundoc(currdoc, w->getGraphicView(), this);
//execute plugin
    LC_UndoSection undo(currdoc);
    plugin->execComm(&pligundoc, this, action->data().toString());
//TODO call update view
w->getGraphicView()->redraw();
}


/**
 * Destructor.
 */
QC_ApplicationWindow::~QC_ApplicationWindow() {
    RS_DEBUG->print("QC_ApplicationWindow::~QC_ApplicationWindow");

    if (kuubikCurrentLayerSelector != nullptr) {
        kuubikCurrentLayerSelector->setLayerList(nullptr);
    }
    if (kuubikPropertiesPalette != nullptr) {
        kuubikPropertiesPalette->setDocument(nullptr);
    }

    RS_DEBUG->print("QC_ApplicationWindow::~QC_ApplicationWindow: "
                    "deleting dialog factory");

#ifdef _WINDOWS
	qt_ntfs_permission_lookup--; // turn it off again
#endif

    delete dialogFactory;

    RS_DEBUG->print("QC_ApplicationWindow::~QC_ApplicationWindow: "
                    "deleting dialog factory: OK");
}


/**
 * Close Event. Called when the user tries to close the app.
 */
void QC_ApplicationWindow::closeEvent(QCloseEvent* ce)
{
    RS_DEBUG->print("QC_ApplicationWindow::closeEvent()");

    queryExit(false) ? ce->accept() : ce->ignore();

    RS_DEBUG->print("QC_ApplicationWindow::closeEvent(): OK");
}

void QC_ApplicationWindow::dropEvent(QDropEvent* event)
{
    event->acceptProposedAction();

    //limit maximum number of dropped files to be opened
    unsigned counts=0;
    for(QUrl const& url: event->mimeData()->urls()) {
        const QString &fileName = url.toLocalFile();
        if(QFileInfo(fileName).exists() && fileName.endsWith(R"(.dxf)", Qt::CaseInsensitive)){
            slotFileOpen(fileName);
            if(++counts>32) return;
        }
    }
}

void 	QC_ApplicationWindow::dragEnterEvent(QDragEnterEvent * event)
{
    if (event->mimeData()->hasUrls()){
        for(QUrl const& url: event->mimeData()->urls()) {
            const QString &fileName = url.toLocalFile();
            if(QFileInfo(fileName).exists() && fileName.endsWith(R"(.dxf)", Qt::CaseInsensitive)){
                event->acceptProposedAction();
                return;
            }
        }
    }
}

/**
 * @return Pointer to the currently active MDI Window or nullptr if no
 * MDI Window is active.
 */
QC_MDIWindow const* QC_ApplicationWindow::getMDIWindow() const{
    if (mdiAreaCAD) {
        QMdiSubWindow* w=mdiAreaCAD->currentSubWindow();
        if(w) {
            return qobject_cast<QC_MDIWindow*>(w);
        }
    }
    return nullptr;
}

QC_MDIWindow* QC_ApplicationWindow::getMDIWindow(){
    if (mdiAreaCAD) {
        QMdiSubWindow* w=mdiAreaCAD->currentSubWindow();
        if(w) {
            return qobject_cast<QC_MDIWindow*>(w);
        }
    }
    return nullptr;
}

void QC_ApplicationWindow::setPreviousZoomEnable(bool enable){
    previousZoomEnable=enable;
    if(previousZoom){
        previousZoom->setEnabled(enable);
    }
}

void QC_ApplicationWindow::setUndoEnable(bool enable){
    undoEnable=enable;
    if(undoButton){
        undoButton->setEnabled(enable);
    }
}

void QC_ApplicationWindow::setRedoEnable(bool enable){
    redoEnable=enable;
    if(redoButton){
        redoButton->setEnabled(enable);
    }
}

void QC_ApplicationWindow::slotEnableActions(bool enable) {
    if(previousZoom){
        previousZoom->setEnabled(enable&& previousZoomEnable);
        undoButton->setEnabled(enable&& undoEnable);
        redoButton->setEnabled(enable&& redoEnable);
    }
}

void QC_ApplicationWindow::slotUpdateActiveLayer()
{
    if(layerWidget&&m_pActiveLayerName)
        m_pActiveLayerName->activeLayerChanged(layerWidget->getActiveName());
}

/**
 * Initializes the global application settings from the
 * config file (unix, mac) or registry (windows).
 */
void QC_ApplicationWindow::initSettings()
{
    RS_DEBUG->print("QC_ApplicationWindow::initSettings()");

    QSettings settings;

    settings.beginGroup("Geometry");
    restoreState(settings.value("StateOfWidgets", "").toByteArray());
    dock_areas.left->setChecked(settings.value("LeftDockArea", 0).toBool());
    dock_areas.right->setChecked(settings.value("RightDockArea", 1).toBool());
    dock_areas.top->setChecked(settings.value("TopDockArea", 0).toBool());
    dock_areas.bottom->setChecked(settings.value("BottomDockArea", 0).toBool());
    dock_areas.floating->setChecked(settings.value("FloatingDockwidgets", 0).toBool());
    settings.endGroup();

    settings.beginGroup("Widgets");

    int allow_style = settings.value("AllowStyle", 0).toInt();
    if (allow_style)
    {
        QString style = settings.value("Style", "").toString();
        QApplication::setStyle(QStyleFactory::create(style));
    }

    QString sheet_path = settings.value("StyleSheet", "").toString();
    if (loadStyleSheet(sheet_path))
        style_sheet_path = sheet_path;
    settings.endGroup();

    a_map["ViewDraft"]->setChecked(settings.value("Appearance/DraftMode", 0).toBool());
}

void QC_ApplicationWindow::createKuubikWorkspaceMenu()
{
    auto* viewMenu = menuBar()->findChild<QMenu*>("view_menu");
    if (viewMenu == nullptr) {
        return;
    }

    viewMenu->addSeparator();
    auto* workspaceMenu = viewMenu->addMenu(tr("Workspace"));
    workspaceMenu->setObjectName("kuubik_workspace_menu");

    auto* workspaceGroup = new QActionGroup(this);
    workspaceGroup->setExclusive(true);
    kuubikWorkspaceAction = workspaceMenu->addAction(tr("Kuubik workspace"));
    classicWorkspaceAction = workspaceMenu->addAction(tr("Classic workspace"));
    kuubikWorkspaceAction->setObjectName(QStringLiteral("KuubikWorkspace"));
    classicWorkspaceAction->setObjectName(QStringLiteral("ClassicWorkspace"));
    classicWorkspaceAction->setShortcut(QKeySequence(QStringLiteral("Ctrl+Alt+Shift+C")));
    kuubikWorkspaceAction->setCheckable(true);
    classicWorkspaceAction->setCheckable(true);
    workspaceGroup->addAction(kuubikWorkspaceAction);
    workspaceGroup->addAction(classicWorkspaceAction);

    workspaceMenu->addSeparator();
    paletteLeftAction = workspaceMenu->addAction(tr("Palette Left"));
    paletteRightAction = workspaceMenu->addAction(tr("Palette Right"));
    paletteLeftAction->setCheckable(true);
    paletteRightAction->setCheckable(true);
    auto* paletteGroup = new QActionGroup(this);
    paletteGroup->setExclusive(true);
    paletteGroup->addAction(paletteLeftAction);
    paletteGroup->addAction(paletteRightAction);
    workspaceMenu->addSeparator();
    auto* resetAction = workspaceMenu->addAction(tr("Reset Kuubik Workspace"));

    connect(kuubikWorkspaceAction, &QAction::triggered, this, [this]() {
        applyKuubikWorkspace(true);
    });
    connect(classicWorkspaceAction, &QAction::triggered, this, [this]() {
        applyClassicWorkspace();
    });
    connect(paletteLeftAction, &QAction::triggered, this, [this]() {
        setKuubikPaletteSide(Qt::LeftDockWidgetArea);
    });
    connect(paletteRightAction, &QAction::triggered, this, [this]() {
        setKuubikPaletteSide(Qt::RightDockWidgetArea);
    });
    connect(resetAction, &QAction::triggered, this, [this]() {
        applyKuubikWorkspace(true);
    });
    if (kuubikRibbon != nullptr) {
        kuubikRibbon->setWorkspaceActions(kuubikWorkspaceAction, classicWorkspaceAction);
    }
}

void QC_ApplicationWindow::createKuubikStatusControls()
{
    struct StatusItem {
        QString key;
        QString label;
        QWidget* widget;
        bool defaultVisible;
    };

    QStatusBar* bar = statusBar();
    bar->setObjectName(QStringLiteral("kuubikStatusBar"));
    bar->setSizeGripEnabled(false);

    QSettings statusSettings;
    constexpr int referencePhaseVersion = 3;
    const int installedReferencePhase = statusSettings.value(
        QStringLiteral("KuubikStatus/ReferencePhaseVersion"), 0).toInt();
    if (installedReferencePhase < 1) {
        // The first five pages of the approved visual reference establish the
        // default coordinate/model/grid cluster. Apply this once so later
        // user customization remains authoritative.
        statusSettings.setValue(
            QStringLiteral("KuubikStatus/Visible/Coordinates"), true);
        statusSettings.setValue(
            QStringLiteral("KuubikStatus/Visible/ModelSpace"), true);
        statusSettings.setValue(
            QStringLiteral("KuubikStatus/Visible/Grid"), true);
    }
    if (installedReferencePhase < 2) {
        // Pages 6-11 add the precision-drafting controls. Do not revisit the
        // page 1-5 choices when migrating an existing profile.
        for (const QString& key : {
                 QStringLiteral("SnapMode"),
                 QStringLiteral("InferConstraints"),
                 QStringLiteral("DynamicInput"),
                 QStringLiteral("OrthoMode"),
                 QStringLiteral("SnapAngle"),
                 QStringLiteral("IsometricDrafting")}) {
            statusSettings.setValue(
                QStringLiteral("KuubikStatus/Visible/") + key, true);
        }
        statusSettings.setValue(
            QStringLiteral("KuubikStatus/DynamicShowDistance"), true);
        statusSettings.setValue(
            QStringLiteral("KuubikStatus/DynamicShowAngle"), true);
    }
    if (installedReferencePhase < 3) {
        // Pages 12-13 make the already-present OSNAP and OTRACK controls part
        // of the default precision row. Missing keys receive the new default;
        // an existing explicit hidden choice remains authoritative.
        applyPhaseThreeVisibilityDefaults(
            [&statusSettings](const QString& key) {
                return statusSettings.contains(key);
            },
            [&statusSettings](const QString& key, bool value) {
                statusSettings.setValue(key, value);
            });
    }
    statusSettings.setValue(QStringLiteral("KuubikStatus/ReferencePhaseVersion"),
                            referencePhaseVersion);

    QList<StatusItem> statusItems;
    const auto registerStatusItem = [bar, &statusItems](QWidget* widget,
                                                        const QString& key,
                                                        const QString& label,
                                                        bool defaultVisible,
                                                        bool addToBar = true) {
        widget->setProperty("kuubikStatusItem", true);
        widget->setProperty("kuubikStatusKey", key);
        widget->setProperty("kuubikStatusDefaultVisible", defaultVisible);
        widget->setProperty("kuubikStatusOrder", statusItems.size());
        statusItems.append({key, label, widget, defaultVisible});
        if (addToBar) {
            bar->addPermanentWidget(widget);
        }
    };

    if (coordinateWidget != nullptr) {
        // Keep the coordinate readout in the same right-aligned drafting
        // cluster as MODEL and GRID, matching the approved reference pages.
        bar->removeWidget(coordinateWidget);
        bar->addPermanentWidget(coordinateWidget);
        coordinateWidget->setProperty("kuubikReferencePage", 3);
        coordinateWidget->setToolTip(
            tr("Coordinates\nRight-click to change the coordinate format."));
        coordinateWidget->setAccessibleName(tr("Cursor coordinates"));
        registerStatusItem(coordinateWidget, QStringLiteral("Coordinates"),
                           tr("Coordinates"), true, false);

        auto* coordinateMenu = new QMenu(coordinateWidget);
        coordinateMenu->setObjectName(QStringLiteral("kuubikCoordinateMenu"));
        auto* coordinateGroup = new QActionGroup(coordinateMenu);
        coordinateGroup->setExclusive(true);
        const QList<QPair<QString, QString>> coordinateModes {
            {QStringLiteral("absolute-cartesian"), tr("Absolute Cartesian")},
            {QStringLiteral("relative-cartesian"), tr("Relative Cartesian")},
            {QStringLiteral("absolute-polar"), tr("Absolute Polar")},
            {QStringLiteral("relative-polar"), tr("Relative Polar")}
        };
        const QString savedMode = QSettings().value(
            QStringLiteral("KuubikStatus/CoordinateMode"),
            QStringLiteral("absolute-cartesian")).toString();
        for (const auto& mode : coordinateModes) {
            QAction* modeAction = coordinateMenu->addAction(mode.second);
            modeAction->setObjectName(QStringLiteral("kuubikCoordinateMode_%1").arg(mode.first));
            modeAction->setData(mode.first);
            modeAction->setCheckable(true);
            modeAction->setChecked(mode.first == savedMode);
            coordinateGroup->addAction(modeAction);
            connect(modeAction, &QAction::triggered, this, [this, mode]() {
                QSettings().setValue(QStringLiteral("KuubikStatus/CoordinateMode"),
                                     mode.first);
                configureKuubikCoordinateWidget(true);
            });
        }
        coordinateWidget->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(coordinateWidget, &QWidget::customContextMenuRequested,
                coordinateMenu, [this, coordinateMenu](const QPoint& pos) {
                    coordinateMenu->popup(coordinateWidget->mapToGlobal(pos));
                });
    }

    auto* modelLabel = new QLabel(QStringLiteral("MODEL"), statusBar());
    modelLabel->setObjectName(QStringLiteral("kuubikModelStatus"));
    modelLabel->setProperty("kuubikReferencePage", 4);
    modelLabel->setAlignment(Qt::AlignCenter);
    modelLabel->setToolTip(
        tr("Model space\nLibreCAD drawings use model space."));
    modelLabel->setAccessibleName(tr("Model space"));
    registerStatusItem(modelLabel, QStringLiteral("ModelSpace"),
                       tr("Model Space"), true);

    const auto addSettingsContextMenu = [this](QToolButton* button,
                                                const QString& settingsKey,
                                                const QString& label) {
        QAction* settingsAction = a_map.value(settingsKey, nullptr);
        if (button == nullptr || settingsAction == nullptr) return;
        auto* menu = new QMenu(button);
        menu->setObjectName(QStringLiteral("kuubikStatusSettingsMenu"));
        QAction* openSettings = menu->addAction(label);
        connect(openSettings, &QAction::triggered, settingsAction,
                [settingsAction]() { settingsAction->trigger(); });
        button->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(button, &QWidget::customContextMenuRequested, menu,
                [button, menu](const QPoint& pos) {
                    menu->popup(button->mapToGlobal(pos));
                });
    };

    const auto appendNativeShortcut = [this](const QString& actionKey,
                                              const QKeySequence& shortcut) {
        QAction* action = a_map.value(actionKey, nullptr);
        if (action == nullptr) return;
        QList<QKeySequence> shortcuts = action->shortcuts();
        if (!shortcuts.contains(shortcut)) shortcuts.append(shortcut);
        action->setShortcuts(shortcuts);
    };

    const auto addNativeToggle = [this, &registerStatusItem](
                                     const QString& actionKey,
                                     const QString& visibilityKey,
                                     const QString& label,
                                     const QString& iconPath,
                                     const QString& tooltip) {
        QAction* action = a_map.value(actionKey, nullptr);
        if (action == nullptr) return static_cast<QToolButton*>(nullptr);
        auto* button = new QToolButton(statusBar());
        button->setObjectName(QStringLiteral("kuubikStatusToggle"));
        button->setProperty("kuubikActionKey", actionKey);
        button->setProperty("kuubikBindingType", QStringLiteral("direct-action"));
        button->setProperty("kuubikIconPath", iconPath);
        button->setText(label);
        button->setIcon(QIcon(iconPath));
        button->setIconSize(QSize(18, 18));
        button->setToolButtonStyle(Qt::ToolButtonIconOnly);
        button->setAutoRaise(false);
        button->setCheckable(action->isCheckable());
        button->setChecked(action->isChecked());
        button->setEnabled(action->isEnabled());
        button->setToolTip(tooltip);
        button->setAccessibleName(tooltip);
        // Do not use setDefaultAction() here. QAction::changed would then copy
        // LibreCAD's action icon back onto the button whenever its checked
        // state changes. Keep the native behavior, but own the Kuubik visual.
        const QMetaObject::Connection nativeConnection = connect(
            button, &QToolButton::clicked, action,
            [action](bool) { action->trigger(); });
        button->setProperty("kuubikNativeActionBound",
                            static_cast<bool>(nativeConnection));
        connect(action, &QAction::changed, button,
                [action, button, iconPath]() {
                    const QSignalBlocker blocker(button);
                    button->setCheckable(action->isCheckable());
                    button->setChecked(action->isChecked());
                    button->setEnabled(action->isEnabled());
                    button->setIcon(QIcon(iconPath));
                });
        registerStatusItem(button, visibilityKey, tooltip, true);
        return button;
    };

    QToolButton* gridButton = addNativeToggle(
        QStringLiteral("ViewGrid"), QStringLiteral("Grid"),
        QStringLiteral("GRID"), QStringLiteral(":/icons/kuubik/view/status-grid.svg"),
        tr("Grid display"));
    if (gridButton != nullptr) {
        gridButton->setProperty("kuubikReferencePage", 5);
        const auto updateGridTooltip = [gridButton]() {
            gridButton->setToolTip(
                gridButton->isChecked()
                    ? tr("Grid display - On (F7)\nRight-click for Grid Settings")
                    : tr("Grid display - Off (F7)\nRight-click for Grid Settings"));
        };
        updateGridTooltip();
        connect(gridButton, &QToolButton::toggled, gridButton,
                [updateGridTooltip](bool) { updateGridTooltip(); });
    }
    addSettingsContextMenu(gridButton, QStringLiteral("OptionsDrawing"),
                           tr("Grid Settings..."));
    appendNativeShortcut(QStringLiteral("ViewGrid"),
                         QKeySequence(Qt::Key_F7));

    QToolButton* snapButton = addNativeToggle(
        QStringLiteral("SnapGrid"), QStringLiteral("SnapMode"),
        QStringLiteral("SNAP"), QStringLiteral(":/icons/kuubik/view/status-snap.svg"),
        tr("Snap mode - Grid Snap (F9)"));
    QAction* snapGridModeAction = nullptr;
    QAction* polarSnapModeAction = nullptr;
    QMenu* snapModeMenu = nullptr;
    if (snapButton != nullptr) {
        snapButton->setObjectName(QStringLiteral("kuubikStatusSplitToggle"));
        snapButton->setProperty("kuubikReferencePage", 6);
        snapButton->setPopupMode(QToolButton::MenuButtonPopup);
        snapModeMenu = new QMenu(snapButton);
        snapModeMenu->setObjectName(QStringLiteral("kuubikSnapModeMenu"));
        auto* snapModeGroup = new QActionGroup(snapModeMenu);
        snapModeGroup->setExclusive(true);
        snapGridModeAction = snapModeMenu->addAction(tr("Grid Snap"));
        snapGridModeAction->setObjectName(
            QStringLiteral("kuubikSnapModeGrid"));
        snapGridModeAction->setCheckable(true);
        snapModeGroup->addAction(snapGridModeAction);
        polarSnapModeAction = snapModeMenu->addAction(tr("Polar Snap"));
        polarSnapModeAction->setObjectName(
            QStringLiteral("kuubikSnapModePolar"));
        polarSnapModeAction->setCheckable(true);
        snapModeGroup->addAction(polarSnapModeAction);
        snapModeMenu->addSeparator();
        if (QAction* settingsAction = a_map.value(
                QStringLiteral("OptionsGeneral"), nullptr)) {
            QAction* openSettings = snapModeMenu->addAction(
                tr("Snap Settings..."));
            connect(openSettings, &QAction::triggered, settingsAction,
                    [settingsAction]() { settingsAction->trigger(); });
        }
        snapButton->setMenu(snapModeMenu);
    }
    addSettingsContextMenu(snapButton, QStringLiteral("OptionsGeneral"),
                           tr("Snap Settings..."));
    appendNativeShortcut(QStringLiteral("SnapGrid"),
                         QKeySequence(Qt::Key_F9));

    const auto addSnapToggle = [this, &registerStatusItem](
                                   const QString& text, const QString& iconPath,
                                   const QString& visibilityKey,
                                   unsigned snapMask,
                                   bool RS_SnapMode::*flag,
                                   const QString& tooltip) {
        auto* button = new QToolButton(statusBar());
        button->setObjectName(QStringLiteral("kuubikStatusToggle"));
        button->setProperty("kuubikActionKey", visibilityKey);
        button->setProperty("kuubikBindingType", QStringLiteral("snap-mode"));
        button->setProperty("kuubikSnapMask", snapMask);
        button->setText(text);
        button->setIcon(QIcon(iconPath));
        button->setIconSize(QSize(18, 18));
        button->setToolButtonStyle(Qt::ToolButtonIconOnly);
        button->setCheckable(true);
        button->setChecked(actionHandler != nullptr && actionHandler->getSnaps().*flag);
        button->setToolTip(tooltip);
        button->setAccessibleName(tooltip);
        connect(button, &QToolButton::toggled, this, [this, flag](bool checked) {
            if (actionHandler == nullptr) return;
            RS_SnapMode mode = actionHandler->getSnaps();
            mode.*flag = checked;
            actionHandler->slotSetSnaps(mode);
        });
        registerStatusItem(button, visibilityKey, tooltip, true);
        return button;
    };

    QToolButton* orthoButton = addNativeToggle(
        QStringLiteral("RestrictOrthogonal"), QStringLiteral("OrthoMode"),
        QStringLiteral("ORTHO"),
        QStringLiteral(":/icons/kuubik/view/status-ortho.svg"),
        tr("Ortho mode"));
    if (orthoButton != nullptr) {
        orthoButton->setProperty("kuubikReferencePage", 9);
        const auto updateOrthoTooltip = [orthoButton]() {
            orthoButton->setToolTip(
                orthoButton->isChecked()
                    ? tr("Ortho mode - On (F8)")
                    : tr("Ortho mode - Off (F8)"));
        };
        updateOrthoTooltip();
        connect(orthoButton, &QToolButton::toggled, orthoButton,
                [updateOrthoTooltip](bool) { updateOrthoTooltip(); });
    }
    appendNativeShortcut(QStringLiteral("RestrictOrthogonal"),
                         QKeySequence(Qt::Key_F8));

    QToolButton* polarButton = addSnapToggle(
        QStringLiteral("POLAR"),
        QStringLiteral(":/icons/kuubik/view/status-polar.svg"),
        QStringLiteral("SnapAngle"), RS_SnapMode::SnapAngle,
        &RS_SnapMode::snapAngle, tr("Polar tracking"));
    const auto readPolarIncrement = []() {
        auto settingsGuard = RS_SETTINGS->beginGroupGuard("/Snap");
        bool ok = false;
        const double value = RS_SETTINGS->readEntry(
            "/AngleIncrement", "15").toDouble(&ok);
        return ok && value > 0.0 && value <= 180.0 ? value : 15.0;
    };
    const auto writePolarIncrement = [](double increment) {
        auto settingsGuard = RS_SETTINGS->beginGroupGuard("/Snap");
        RS_SETTINGS->writeEntry("/AngleIncrement",
                                QString::number(increment, 'g', 8));
    };
    QMenu* polarMenu = nullptr;
    if (polarButton != nullptr) {
        polarButton->setObjectName(QStringLiteral("kuubikStatusSplitToggle"));
        polarButton->setProperty("kuubikReferencePage", 10);
        polarButton->setPopupMode(QToolButton::MenuButtonPopup);
        polarMenu = new QMenu(polarButton);
        polarMenu->setObjectName(QStringLiteral("kuubikPolarMenu"));
        auto* polarGroup = new QActionGroup(polarMenu);
        polarGroup->setExclusive(true);
        for (double increment : {90.0, 45.0, 30.0, 22.5,
                                 18.0, 15.0, 10.0, 5.0}) {
            QAction* preset = polarMenu->addAction(
                tr("%1 degree increment").arg(increment, 0, 'g', 4));
            preset->setProperty("kuubikPolarAngle", increment);
            preset->setCheckable(true);
            polarGroup->addAction(preset);
            connect(preset, &QAction::triggered, polarButton,
                    [writePolarIncrement, increment]() {
                        writePolarIncrement(increment);
                    });
        }
        polarMenu->addSeparator();
        QAction* customAngle = polarMenu->addAction(
            tr("Tracking Settings..."));
        customAngle->setObjectName(
            QStringLiteral("kuubikPolarTrackingSettings"));
        connect(customAngle, &QAction::triggered, this,
                [this, readPolarIncrement, writePolarIncrement]() {
            bool accepted = false;
            const double increment = QInputDialog::getDouble(
                this, tr("Polar Tracking Settings"),
                tr("Increment angle (degrees):"), readPolarIncrement(),
                0.1, 180.0, 2, &accepted);
            if (accepted) writePolarIncrement(increment);
        });
        const auto updatePolarTooltip = [polarButton, readPolarIncrement]() {
            polarButton->setToolTip(
                tr("Polar tracking - %1 (F10)\nIncrement: %2 degrees")
                    .arg(polarButton->isChecked() ? tr("On") : tr("Off"))
                    .arg(readPolarIncrement(), 0, 'g', 4));
        };
        updatePolarTooltip();
        connect(polarButton, &QToolButton::toggled, polarButton,
                [updatePolarTooltip](bool) { updatePolarTooltip(); });
        connect(polarMenu, &QMenu::aboutToShow, polarButton,
                [polarMenu, readPolarIncrement, updatePolarTooltip]() {
            const double current = readPolarIncrement();
            for (QAction* action : polarMenu->actions()) {
                if (!action->property("kuubikPolarAngle").isValid()) continue;
                const QSignalBlocker blocker(action);
                action->setChecked(qAbs(
                    action->property("kuubikPolarAngle").toDouble()
                    - current) < 0.0001);
            }
            updatePolarTooltip();
        });
        polarButton->setMenu(polarMenu);

        auto* polarShortcut = new QShortcut(QKeySequence(Qt::Key_F10), this);
        polarShortcut->setObjectName(QStringLiteral("kuubikShortcutF10"));
        polarShortcut->setContext(Qt::ApplicationShortcut);
        connect(polarShortcut, &QShortcut::activated, polarButton,
                [polarButton]() { polarButton->click(); });
    }

    if (snapButton != nullptr && polarButton != nullptr) {
        QAction* nativeGridSnap = a_map.value(QStringLiteral("SnapGrid"),
                                              nullptr);
        connect(snapButton, &QToolButton::toggled, this,
                [polarButton](bool checked) {
            if (checked && polarButton->isChecked()) {
                polarButton->setChecked(false);
            }
        });
        connect(polarButton, &QToolButton::toggled, this,
                [nativeGridSnap](bool checked) {
            if (checked && nativeGridSnap != nullptr
                && nativeGridSnap->isChecked()) {
                nativeGridSnap->trigger();
            }
        });
        if (nativeGridSnap != nullptr) {
            connect(nativeGridSnap, &QAction::toggled, polarButton,
                    [polarButton](bool checked) {
                if (checked && polarButton->isChecked()) {
                    polarButton->setChecked(false);
                }
            });
        }
        if (snapGridModeAction != nullptr) {
            connect(snapGridModeAction, &QAction::triggered, snapButton,
                    [nativeGridSnap, polarButton]() {
                if (polarButton->isChecked()) polarButton->setChecked(false);
                if (nativeGridSnap != nullptr
                    && !nativeGridSnap->isChecked()) {
                    nativeGridSnap->trigger();
                }
            });
        }
        if (polarSnapModeAction != nullptr) {
            connect(polarSnapModeAction, &QAction::triggered, polarButton,
                    [nativeGridSnap, polarButton]() {
                if (nativeGridSnap != nullptr
                    && nativeGridSnap->isChecked()) {
                    nativeGridSnap->trigger();
                }
                if (!polarButton->isChecked()) polarButton->setChecked(true);
            });
        }
        if (snapModeMenu != nullptr) {
            connect(snapModeMenu, &QMenu::aboutToShow, snapButton,
                    [this, snapGridModeAction, polarSnapModeAction]() {
                if (actionHandler == nullptr) return;
                const RS_SnapMode mode = actionHandler->getSnaps();
                const QSignalBlocker gridBlocker(snapGridModeAction);
                const QSignalBlocker polarBlocker(polarSnapModeAction);
                snapGridModeAction->setChecked(mode.snapGrid);
                polarSnapModeAction->setChecked(mode.snapAngle);
            });
        }
    }

    auto* isometricButton = new QToolButton(statusBar());
    isometricButton->setObjectName(QStringLiteral("kuubikIsometricButton"));
    isometricButton->setProperty("kuubikActionKey",
                                 QStringLiteral("IsometricDrafting"));
    isometricButton->setProperty("kuubikBindingType",
                                 QStringLiteral("document-setting"));
    isometricButton->setProperty("kuubikReferencePage", 11);
    isometricButton->setText(QStringLiteral("ISO"));
    isometricButton->setIcon(QIcon(
        QStringLiteral(":/icons/kuubik/view/status-isometric.svg")));
    isometricButton->setIconSize(QSize(18, 18));
    isometricButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
    isometricButton->setPopupMode(QToolButton::MenuButtonPopup);
    isometricButton->setCheckable(true);
    isometricButton->setAutoRaise(false);
    isometricButton->setAccessibleName(tr("Isometric drafting"));
    auto* isometricMenu = new QMenu(isometricButton);
    isometricMenu->setObjectName(QStringLiteral("kuubikIsometricMenu"));
    auto* isometricGroup = new QActionGroup(isometricMenu);
    isometricGroup->setExclusive(true);
    for (const auto& plane : {
             qMakePair(RS2::LeftCrosshair, tr("Isoplane Left")),
             qMakePair(RS2::TopCrosshair, tr("Isoplane Top")),
             qMakePair(RS2::RightCrosshair, tr("Isoplane Right"))}) {
        QAction* planeAction = isometricMenu->addAction(plane.second);
        planeAction->setProperty("kuubikIsoplane",
                                 static_cast<int>(plane.first));
        planeAction->setCheckable(true);
        isometricGroup->addAction(planeAction);
    }
    isometricButton->setMenu(isometricMenu);
    const auto applyIsometricMode = [this](bool enabled,
                                            RS2::CrosshairType plane) {
        QC_MDIWindow* mdi = getMDIWindow();
        RS_Graphic* graphic = mdi == nullptr ? nullptr : mdi->getGraphic();
        QG_GraphicView* view = mdi == nullptr ? nullptr
                                              : mdi->getGraphicView();
        if (graphic == nullptr || view == nullptr) return;
        graphic->setIsometricGrid(enabled);
        const RS2::CrosshairType appliedPlane = enabled
            ? plane : RS2::OrthogonalCrosshair;
        graphic->setCrosshairType(appliedPlane);
        view->setCrosshairType(appliedPlane);
        view->redraw(RS2::RedrawAll);
    };
    const auto syncIsometricUi = [this, isometricButton, isometricMenu]() {
        QC_MDIWindow* mdi = getMDIWindow();
        RS_Graphic* graphic = mdi == nullptr ? nullptr : mdi->getGraphic();
        const bool enabled = graphic != nullptr
                             && graphic->isIsometricGrid();
        const RS2::CrosshairType plane = graphic == nullptr
            ? RS2::OrthogonalCrosshair : graphic->getCrosshairType();
        {
            const QSignalBlocker blocker(isometricButton);
            isometricButton->setEnabled(graphic != nullptr);
            isometricButton->setChecked(enabled);
        }
        QString planeName = tr("Off");
        for (QAction* action : isometricMenu->actions()) {
            const bool matches = enabled
                && action->property("kuubikIsoplane").toInt()
                       == static_cast<int>(plane);
            const QSignalBlocker blocker(action);
            action->setChecked(matches);
            if (matches) planeName = action->text();
        }
        isometricButton->setToolTip(
            tr("Isometric drafting - %1\nF5 or Ctrl+E cycles isoplanes")
                .arg(planeName));
    };
    connect(isometricButton, &QToolButton::toggled, this,
            [this, applyIsometricMode, syncIsometricUi](bool checked) {
        QC_MDIWindow* mdi = getMDIWindow();
        RS_Graphic* graphic = mdi == nullptr ? nullptr : mdi->getGraphic();
        RS2::CrosshairType plane = graphic == nullptr
            ? RS2::LeftCrosshair : graphic->getCrosshairType();
        if (plane == RS2::OrthogonalCrosshair) plane = RS2::LeftCrosshair;
        applyIsometricMode(checked, plane);
        syncIsometricUi();
    });
    for (QAction* action : isometricMenu->actions()) {
        connect(action, &QAction::triggered, this,
                [action, isometricButton, applyIsometricMode,
                 syncIsometricUi]() {
            const auto plane = static_cast<RS2::CrosshairType>(
                action->property("kuubikIsoplane").toInt());
            {
                const QSignalBlocker blocker(isometricButton);
                isometricButton->setChecked(true);
            }
            applyIsometricMode(true, plane);
            syncIsometricUi();
        });
    }
    connect(isometricMenu, &QMenu::aboutToShow, isometricButton,
            syncIsometricUi);
    connect(this, &QC_ApplicationWindow::windowsChanged, isometricButton,
            [syncIsometricUi](bool) { syncIsometricUi(); });
    const auto cycleIsoplane = [this, applyIsometricMode, syncIsometricUi]() {
        QC_MDIWindow* mdi = getMDIWindow();
        RS_Graphic* graphic = mdi == nullptr ? nullptr : mdi->getGraphic();
        if (graphic == nullptr) return;
        RS2::CrosshairType next = RS2::LeftCrosshair;
        if (graphic->isIsometricGrid()) {
            switch (graphic->getCrosshairType()) {
            case RS2::LeftCrosshair: next = RS2::TopCrosshair; break;
            case RS2::TopCrosshair: next = RS2::RightCrosshair; break;
            default: next = RS2::LeftCrosshair; break;
            }
        }
        applyIsometricMode(true, next);
        syncIsometricUi();
    };
    auto* isometricF5 = new QShortcut(QKeySequence(Qt::Key_F5), this);
    isometricF5->setObjectName(QStringLiteral("kuubikShortcutF5"));
    isometricF5->setContext(Qt::ApplicationShortcut);
    connect(isometricF5, &QShortcut::activated, isometricButton,
            cycleIsoplane);
    auto* isometricCtrlE = new QShortcut(
        QKeySequence(Qt::CTRL | Qt::Key_E), this);
    isometricCtrlE->setObjectName(QStringLiteral("kuubikShortcutCtrlE"));
    isometricCtrlE->setContext(Qt::ApplicationShortcut);
    connect(isometricCtrlE, &QShortcut::activated, isometricButton,
            cycleIsoplane);
    registerStatusItem(isometricButton, QStringLiteral("IsometricDrafting"),
                       tr("Isometric Drafting"), true);
    syncIsometricUi();

    auto* osnapButton = new QToolButton(statusBar());
    osnapButton->setObjectName(QStringLiteral("kuubikOsnapButton"));
    osnapButton->setProperty("kuubikActionKey", QStringLiteral("ObjectSnapMenu"));
    osnapButton->setProperty("kuubikBindingType", QStringLiteral("object-snap-menu"));
    osnapButton->setText(QStringLiteral("OSNAP"));
    osnapButton->setIcon(QIcon(QStringLiteral(":/icons/kuubik/view/status-osnap.svg")));
    osnapButton->setIconSize(QSize(18, 18));
    osnapButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
    osnapButton->setPopupMode(QToolButton::MenuButtonPopup);
    osnapButton->setAutoRaise(false);
    osnapButton->setCheckable(true);
    osnapButton->setAccessibleName(tr("Object Snap modes"));
    osnapButton->setToolTip(tr("Object Snap modes (F3)"));
    osnapButton->setProperty("kuubikReferencePage", 13);

    auto* osnapMenu = new QMenu(osnapButton);
    osnapMenu->setObjectName(QStringLiteral("kuubikOsnapMenu"));
    const auto addNativeSnap = [this, osnapMenu](const QString& key,
                                                 const QString& label,
                                                 unsigned snapMask) {
        if (QAction* nativeAction = a_map.value(key, nullptr)) {
            QIcon icon = KuubikIconRegistry::iconForAction(key);
            if (icon.isNull()) icon = nativeAction->icon();
            auto* menuAction = osnapMenu->addAction(icon, label);
            menuAction->setProperty("kuubikSnapMask", snapMask);
            menuAction->setCheckable(true);
            menuAction->setChecked(nativeAction->isChecked());
            connect(menuAction, &QAction::triggered, nativeAction,
                    [nativeAction] { nativeAction->trigger(); });
            connect(nativeAction, &QAction::changed, menuAction,
                    [nativeAction, menuAction] {
                        const QSignalBlocker blocker(menuAction);
                        menuAction->setChecked(nativeAction->isChecked());
                        menuAction->setEnabled(nativeAction->isEnabled());
                    });
        }
    };
    const auto addExtendedSnap = [this, osnapMenu](const QString& label,
                                                   const QString& icon,
                                                   unsigned snapMask,
                                                   bool RS_SnapMode::*flag) {
        auto* menuAction = osnapMenu->addAction(QIcon(icon), label);
        menuAction->setProperty("kuubikSnapMask", snapMask);
        menuAction->setCheckable(true);
        menuAction->setChecked(actionHandler != nullptr
                               && actionHandler->getSnaps().*flag);
        connect(menuAction, &QAction::toggled, this,
                [this, flag](bool checked) {
                    if (actionHandler == nullptr) return;
                    RS_SnapMode mode = actionHandler->getSnaps();
                    mode.*flag = checked;
                    actionHandler->slotSetSnaps(mode);
                });
    };
    addNativeSnap(QStringLiteral("SnapEnd"), tr("Endpoint"), RS_SnapMode::SnapEndpoint);
    addNativeSnap(QStringLiteral("SnapMiddle"), tr("Midpoint"), RS_SnapMode::SnapMiddle);
    addNativeSnap(QStringLiteral("SnapCenter"), tr("Center"), RS_SnapMode::SnapCenter);
    addExtendedSnap(tr("Geometric Center"), QStringLiteral(":/icons/kuubik/view/snap-geometric-center.svg"), RS_SnapMode::SnapGeometricCenter, &RS_SnapMode::snapGeometricCenter);
    addExtendedSnap(tr("Node"), QStringLiteral(":/icons/kuubik/view/snap-node.svg"), RS_SnapMode::SnapNode, &RS_SnapMode::snapNode);
    addExtendedSnap(tr("Quadrant"), QStringLiteral(":/icons/kuubik/view/snap-quadrant.svg"), RS_SnapMode::SnapQuadrant, &RS_SnapMode::snapQuadrant);
    addNativeSnap(QStringLiteral("SnapIntersection"), tr("Intersection"), RS_SnapMode::SnapIntersection);
    addExtendedSnap(tr("Extension"), QStringLiteral(":/icons/kuubik/view/snap-extension.svg"), RS_SnapMode::SnapExtension, &RS_SnapMode::snapExtension);
    addExtendedSnap(tr("Insertion"), QStringLiteral(":/icons/kuubik/view/snap-insertion.svg"), RS_SnapMode::SnapInsertion, &RS_SnapMode::snapInsertion);
    addExtendedSnap(tr("Perpendicular"), QStringLiteral(":/icons/kuubik/view/snap-perpendicular.svg"), RS_SnapMode::SnapPerpendicular, &RS_SnapMode::snapPerpendicular);
    addExtendedSnap(tr("Tangent"), QStringLiteral(":/icons/kuubik/view/snap-tangent.svg"), RS_SnapMode::SnapTangent, &RS_SnapMode::snapTangent);
    addNativeSnap(QStringLiteral("SnapEntity"), tr("Nearest"), RS_SnapMode::SnapOnEntity);
    addExtendedSnap(tr("Apparent Intersection"), QStringLiteral(":/icons/kuubik/view/snap-apparent.svg"), RS_SnapMode::SnapApparentIntersection, &RS_SnapMode::snapApparentIntersection);
    addExtendedSnap(tr("Parallel"), QStringLiteral(":/icons/kuubik/view/snap-parallel.svg"), RS_SnapMode::SnapParallel, &RS_SnapMode::snapParallel);
    osnapMenu->addSeparator();
    if (QAction* settings = a_map.value(QStringLiteral("OptionsGeneral"),
                                        nullptr)) {
        auto* settingsAction = osnapMenu->addAction(
            tr("Object Snap Settings..."));
        connect(settingsAction, &QAction::triggered, settings,
                [settings] { settings->trigger(); });
    }
    osnapButton->setMenu(osnapMenu);

    constexpr unsigned objectSnapMask = RS_SnapMode::SnapEndpoint
        | RS_SnapMode::SnapMiddle | RS_SnapMode::SnapCenter
        | RS_SnapMode::SnapIntersection | RS_SnapMode::SnapOnEntity
        | RS_SnapMode::SnapDistance | RS_SnapMode::SnapQuadrant
        | RS_SnapMode::SnapNode | RS_SnapMode::SnapInsertion
        | RS_SnapMode::SnapPerpendicular | RS_SnapMode::SnapTangent
        | RS_SnapMode::SnapGeometricCenter
        | RS_SnapMode::SnapApparentIntersection
        | RS_SnapMode::SnapExtension | RS_SnapMode::SnapParallel;
    const QString savedOsnapKey = QStringLiteral(
        "KuubikStatus/SavedObjectSnapMode");
    const auto applyObjectSnapBits = [](RS_SnapMode& mode, unsigned bits) {
        const RS_SnapMode saved = RS_SnapMode::fromInt(bits);
        mode.snapEndpoint = saved.snapEndpoint;
        mode.snapMiddle = saved.snapMiddle;
        mode.snapCenter = saved.snapCenter;
        mode.snapIntersection = saved.snapIntersection;
        mode.snapOnEntity = saved.snapOnEntity;
        mode.snapDistance = saved.snapDistance;
        mode.snapQuadrant = saved.snapQuadrant;
        mode.snapNode = saved.snapNode;
        mode.snapInsertion = saved.snapInsertion;
        mode.snapPerpendicular = saved.snapPerpendicular;
        mode.snapTangent = saved.snapTangent;
        mode.snapGeometricCenter = saved.snapGeometricCenter;
        mode.snapApparentIntersection = saved.snapApparentIntersection;
        mode.snapExtension = saved.snapExtension;
        mode.snapParallel = saved.snapParallel;
    };
    const unsigned initialObjectSnaps = actionHandler == nullptr ? 0
        : RS_SnapMode::toInt(actionHandler->getSnaps()) & objectSnapMask;
    osnapButton->setProperty(
        "kuubikSavedOsnap",
        initialObjectSnaps != 0
            ? initialObjectSnaps
            : statusSettings.value(
                  savedOsnapKey,
                  RS_SnapMode::SnapEndpoint | RS_SnapMode::SnapMiddle
                      | RS_SnapMode::SnapCenter
                      | RS_SnapMode::SnapIntersection).toUInt());

    const auto syncOsnapUi = [this, osnapButton, osnapMenu]() {
        if (actionHandler == nullptr) return;
        const unsigned snapBits = RS_SnapMode::toInt(actionHandler->getSnaps());
        bool enabled = false;
        for (QAction* action : osnapMenu->actions()) {
            bool validMask = false;
            const unsigned mask = action->property("kuubikSnapMask").toUInt(&validMask);
            if (!validMask || mask == 0) continue;
            const bool checked = (snapBits & mask) != 0;
            const QSignalBlocker blocker(action);
            action->setChecked(checked);
            enabled = enabled || checked;
        }
        const QSignalBlocker blocker(osnapButton);
        osnapButton->setChecked(enabled);
    };
    syncOsnapUi();
    connect(osnapMenu, &QMenu::triggered, this,
            [osnapButton, syncOsnapUi](QAction*) {
                QTimer::singleShot(0, osnapButton, syncOsnapUi);
            });
    connect(osnapButton, &QToolButton::clicked, this,
            [this, osnapButton, syncOsnapUi, savedOsnapKey,
             applyObjectSnapBits, objectSnapMask](bool) {
        if (actionHandler == nullptr) return;
        RS_SnapMode mode = actionHandler->getSnaps();
        const bool enabled = (RS_SnapMode::toInt(mode) & objectSnapMask) != 0;
        if (enabled) {
            const unsigned saved = RS_SnapMode::toInt(mode) & objectSnapMask;
            osnapButton->setProperty("kuubikSavedOsnap", saved);
            QSettings().setValue(savedOsnapKey, saved);
            applyObjectSnapBits(mode, 0);
        } else {
            const unsigned saved = osnapButton->property("kuubikSavedOsnap").toUInt();
            applyObjectSnapBits(
                mode, saved == 0
                    ? RS_SnapMode::SnapEndpoint | RS_SnapMode::SnapMiddle
                          | RS_SnapMode::SnapCenter
                          | RS_SnapMode::SnapIntersection
                    : saved);
        }
        actionHandler->slotSetSnaps(mode);
        syncOsnapUi();
    });
    auto* osnapF3 = new QShortcut(QKeySequence(Qt::Key_F3), this);
    osnapF3->setObjectName(QStringLiteral("kuubikShortcutF3"));
    osnapF3->setContext(Qt::ApplicationShortcut);
    connect(osnapF3, &QShortcut::activated, osnapButton,
            [osnapButton]() { osnapButton->click(); });
    registerStatusItem(osnapButton, QStringLiteral("ObjectSnap"),
                       tr("Object Snap"), true);

    auto* inferButton = new QToolButton(statusBar());
    inferButton->setObjectName(QStringLiteral("kuubikStatusToggle"));
    inferButton->setProperty("kuubikActionKey",
                             QStringLiteral("InferConstraints"));
    inferButton->setProperty("kuubikBindingType",
                             QStringLiteral("inference-bundle"));
    inferButton->setProperty("kuubikReferencePage", 7);
    inferButton->setProperty("kuubikNonParametric", true);
    inferButton->setText(QStringLiteral("INFER"));
    inferButton->setIcon(QIcon(
        QStringLiteral(":/icons/kuubik/view/status-infer.svg")));
    inferButton->setIconSize(QSize(18, 18));
    inferButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
    inferButton->setCheckable(true);
    inferButton->setAutoRaise(false);
    inferButton->setAccessibleName(tr("Geometric inference"));
    const auto inferBundleEnabled = [this]() {
        if (actionHandler == nullptr) return false;
        const RS_SnapMode mode = actionHandler->getSnaps();
        return mode.snapEndpoint && mode.snapPerpendicular
            && mode.snapTangent && mode.snapParallel;
    };
    const auto updateInferTooltip = [inferButton]() {
        inferButton->setToolTip(
            inferButton->isChecked()
                ? tr("Geometric inference - On\nUses native coincident, perpendicular, tangent, and parallel snaps; constraints are not persistent")
                : tr("Geometric inference - Off\nUses native drafting snaps; constraints are not persistent"));
    };
    inferButton->setChecked(inferBundleEnabled());
    updateInferTooltip();
    connect(inferButton, &QToolButton::toggled, this,
            [this, inferButton, syncOsnapUi,
             updateInferTooltip](bool checked) {
        if (actionHandler == nullptr) return;
        RS_SnapMode mode = actionHandler->getSnaps();
        if (checked) {
            QSettings().setValue(
                QStringLiteral("KuubikStatus/InferSavedSnapMode"),
                RS_SnapMode::toInt(mode));
            mode.snapEndpoint = true;
            mode.snapPerpendicular = true;
            mode.snapTangent = true;
            mode.snapParallel = true;
        } else {
            const RS_SnapMode saved = RS_SnapMode::fromInt(
                QSettings().value(
                    QStringLiteral("KuubikStatus/InferSavedSnapMode"),
                    0).toUInt());
            mode.snapEndpoint = saved.snapEndpoint;
            mode.snapPerpendicular = saved.snapPerpendicular;
            mode.snapTangent = saved.snapTangent;
            mode.snapParallel = saved.snapParallel;
        }
        actionHandler->slotSetSnaps(mode);
        {
            const QSignalBlocker blocker(inferButton);
            inferButton->setChecked(checked);
        }
        updateInferTooltip();
        syncOsnapUi();
    });
    connect(osnapMenu, &QMenu::triggered, inferButton,
            [inferButton, inferBundleEnabled, updateInferTooltip](QAction*) {
        QTimer::singleShot(0, inferButton,
                          [inferButton, inferBundleEnabled,
                           updateInferTooltip]() {
            const QSignalBlocker blocker(inferButton);
            inferButton->setChecked(inferBundleEnabled());
            updateInferTooltip();
        });
    });
    registerStatusItem(inferButton, QStringLiteral("InferConstraints"),
                       tr("Infer Constraints (native inference)"), true);

    QToolButton* trackingButton = addSnapToggle(
        QStringLiteral("OTRACK"),
        QStringLiteral(":/icons/kuubik/view/status-otrack.svg"),
        QStringLiteral("SnapTracking"), RS_SnapMode::SnapTracking,
        &RS_SnapMode::snapTracking, tr("Object snap tracking (F11)"));
    if (trackingButton != nullptr) {
        trackingButton->setProperty("kuubikReferencePage", 12);
        auto* trackingF11 = new QShortcut(
            QKeySequence(Qt::Key_F11), this);
        trackingF11->setObjectName(QStringLiteral("kuubikShortcutF11"));
        trackingF11->setContext(Qt::ApplicationShortcut);
        connect(trackingF11, &QShortcut::activated, trackingButton,
                [trackingButton]() { trackingButton->click(); });
    }

    const auto addLocalToggle = [this, &registerStatusItem](
                                    const QString& text, const QString& iconPath,
                                    const QString& key, bool initial,
                                    const QString& tooltip) {
        auto* button = new QToolButton(statusBar());
        button->setObjectName(QStringLiteral("kuubikStatusToggle"));
        button->setProperty("kuubikActionKey", key);
        button->setProperty("kuubikBindingType", QStringLiteral("application-setting"));
        button->setText(text);
        button->setIcon(QIcon(iconPath));
        button->setIconSize(QSize(18, 18));
        button->setToolButtonStyle(Qt::ToolButtonIconOnly);
        button->setCheckable(true);
        button->setChecked(initial);
        button->setToolTip(tooltip);
        button->setAccessibleName(tooltip);
        connect(button, &QToolButton::toggled, this, [this, key](bool checked) {
            QSettings().setValue(QStringLiteral("KuubikStatus/") + key, checked);
            if (key == QStringLiteral("DynamicInput") && !checked) {
                const auto dynamicInputs = findChildren<QLabel*>(
                    QStringLiteral("kuubikDynamicInput"));
                for (QLabel* input : dynamicInputs) input->hide();
            }
        });
        registerStatusItem(button, key, tooltip, true);
        return button;
    };
    QToolButton* dynamicButton = addLocalToggle(
        QStringLiteral("DYN"),
        QStringLiteral(":/icons/kuubik/view/status-dynamic.svg"),
        QStringLiteral("DynamicInput"),
        statusSettings.value(QStringLiteral("KuubikStatus/DynamicInput"),
                             true).toBool(),
        tr("Dynamic input"));
    if (dynamicButton != nullptr) {
        dynamicButton->setProperty("kuubikReferencePage", 8);
        const auto updateDynamicTooltip = [dynamicButton]() {
            dynamicButton->setToolTip(
                dynamicButton->isChecked()
                    ? tr("Dynamic input - On (F12)\nRight-click for settings")
                    : tr("Dynamic input - Off (F12)\nRight-click for settings"));
        };
        updateDynamicTooltip();
        connect(dynamicButton, &QToolButton::toggled, dynamicButton,
                [updateDynamicTooltip](bool) { updateDynamicTooltip(); });

        auto* dynamicMenu = new QMenu(dynamicButton);
        dynamicMenu->setObjectName(QStringLiteral("kuubikDynamicInputMenu"));
        QAction* showDistance = dynamicMenu->addAction(
            tr("Show Distance Field"));
        QAction* showAngle = dynamicMenu->addAction(
            tr("Show Angle Field"));
        showDistance->setObjectName(
            QStringLiteral("kuubikDynamicShowDistance"));
        showAngle->setObjectName(QStringLiteral("kuubikDynamicShowAngle"));
        showDistance->setCheckable(true);
        showAngle->setCheckable(true);
        showDistance->setChecked(statusSettings.value(
            QStringLiteral("KuubikStatus/DynamicShowDistance"), true).toBool());
        showAngle->setChecked(statusSettings.value(
            QStringLiteral("KuubikStatus/DynamicShowAngle"), true).toBool());
        connect(showDistance, &QAction::toggled, dynamicButton,
                [showDistance, showAngle](bool checked) {
            if (!checked && !showAngle->isChecked()) {
                const QSignalBlocker blocker(showDistance);
                showDistance->setChecked(true);
                return;
            }
            QSettings().setValue(
                QStringLiteral("KuubikStatus/DynamicShowDistance"), checked);
        });
        connect(showAngle, &QAction::toggled, dynamicButton,
                [showDistance, showAngle](bool checked) {
            if (!checked && !showDistance->isChecked()) {
                const QSignalBlocker blocker(showAngle);
                showAngle->setChecked(true);
                return;
            }
            QSettings().setValue(
                QStringLiteral("KuubikStatus/DynamicShowAngle"), checked);
        });
        dynamicButton->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(dynamicButton, &QWidget::customContextMenuRequested,
                dynamicMenu, [dynamicButton, dynamicMenu](const QPoint& pos) {
            dynamicMenu->popup(dynamicButton->mapToGlobal(pos));
        });
        auto* dynamicShortcut = new QShortcut(
            QKeySequence(Qt::Key_F12), this);
        dynamicShortcut->setObjectName(QStringLiteral("kuubikShortcutF12"));
        dynamicShortcut->setContext(Qt::ApplicationShortcut);
        connect(dynamicShortcut, &QShortcut::activated, dynamicButton,
                [dynamicButton]() { dynamicButton->click(); });
    }

    if (QAction* draftAction = a_map.value(QStringLiteral("ViewDraft"), nullptr)) {
        auto* lineweightButton = new QToolButton(statusBar());
        lineweightButton->setObjectName(QStringLiteral("kuubikStatusToggle"));
        lineweightButton->setProperty("kuubikActionKey", QStringLiteral("ViewDraft"));
        lineweightButton->setProperty("kuubikBindingType", QStringLiteral("inverse-action"));
        lineweightButton->setText(QStringLiteral("LWT"));
        lineweightButton->setIcon(QIcon(QStringLiteral(":/icons/kuubik/view/status-lineweight.svg")));
        lineweightButton->setIconSize(QSize(18, 18));
        lineweightButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
        lineweightButton->setCheckable(true);
        lineweightButton->setChecked(!draftAction->isChecked());
        lineweightButton->setToolTip(tr("Show lineweight"));
        lineweightButton->setAccessibleName(tr("Show lineweight"));
        connect(lineweightButton, &QToolButton::toggled, draftAction,
                [draftAction](bool showLineweight) {
                    if (draftAction->isChecked() == showLineweight) {
                        draftAction->setChecked(!showLineweight);
                    }
                });
        connect(draftAction, &QAction::changed, lineweightButton,
                [draftAction, lineweightButton]() {
                    const QSignalBlocker blocker(lineweightButton);
                    lineweightButton->setChecked(!draftAction->isChecked());
                });
        registerStatusItem(lineweightButton, QStringLiteral("Lineweight"),
                           tr("LineWeight"), true);
    }

    if (kuubikPropertiesDock != nullptr) {
        auto* quickPropertiesButton = new QToolButton(statusBar());
        quickPropertiesButton->setObjectName(QStringLiteral("kuubikStatusToggle"));
        quickPropertiesButton->setProperty("kuubikActionKey", QStringLiteral("QuickProperties"));
        quickPropertiesButton->setProperty("kuubikBindingType", QStringLiteral("dock-visibility"));
        quickPropertiesButton->setText(QStringLiteral("QP"));
        quickPropertiesButton->setIcon(QIcon(QStringLiteral(":/icons/kuubik/view/status-selection.svg")));
        quickPropertiesButton->setIconSize(QSize(18, 18));
        quickPropertiesButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
        quickPropertiesButton->setCheckable(true);
        quickPropertiesButton->setChecked(statusSettings.value(
            QStringLiteral("KuubikStatus/QuickProperties"), true).toBool());
        quickPropertiesButton->setToolTip(tr("Quick Properties"));
        quickPropertiesButton->setAccessibleName(tr("Quick Properties"));
        connect(quickPropertiesButton, &QToolButton::toggled,
                kuubikPropertiesDock, [this](bool visible) {
                    QSettings().setValue(QStringLiteral("KuubikStatus/QuickProperties"),
                                         visible);
                    if (kuubikPropertiesDock != nullptr) {
                        kuubikPropertiesDock->setVisible(visible);
                    }
                });
        connect(kuubikPropertiesDock, &QDockWidget::visibilityChanged,
                quickPropertiesButton, [quickPropertiesButton](bool visible) {
                    QSettings().setValue(QStringLiteral("KuubikStatus/QuickProperties"),
                                         visible);
                    const QSignalBlocker blocker(quickPropertiesButton);
                    quickPropertiesButton->setChecked(visible);
                });
        registerStatusItem(quickPropertiesButton, QStringLiteral("QuickProperties"),
                           tr("Quick Properties"), true);
    }

    if (QAction* fullScreen = a_map.value(QStringLiteral("Fullscreen"), nullptr)) {
        auto* cleanScreen = new QToolButton(statusBar());
        cleanScreen->setObjectName(QStringLiteral("kuubikStatusToggle"));
        cleanScreen->setProperty("kuubikActionKey", QStringLiteral("Fullscreen"));
        cleanScreen->setProperty("kuubikBindingType", QStringLiteral("direct-action"));
        const QString cleanIconPath = QStringLiteral(
            ":/icons/kuubik/view/status-clean.svg");
        cleanScreen->setProperty("kuubikIconPath", cleanIconPath);
        cleanScreen->setText(QStringLiteral("CLEAN"));
        cleanScreen->setIcon(QIcon(cleanIconPath));
        cleanScreen->setIconSize(QSize(18, 18));
        cleanScreen->setToolButtonStyle(Qt::ToolButtonIconOnly);
        cleanScreen->setCheckable(fullScreen->isCheckable());
        cleanScreen->setChecked(fullScreen->isChecked());
        cleanScreen->setEnabled(fullScreen->isEnabled());
        cleanScreen->setToolTip(tr("Clean screen"));
        cleanScreen->setAccessibleName(tr("Clean screen"));
        const QMetaObject::Connection nativeConnection = connect(
            cleanScreen, &QToolButton::clicked, fullScreen,
            [fullScreen](bool) { fullScreen->trigger(); });
        cleanScreen->setProperty("kuubikNativeActionBound",
                                 static_cast<bool>(nativeConnection));
        connect(fullScreen, &QAction::changed, cleanScreen,
                [fullScreen, cleanScreen, cleanIconPath]() {
                    const QSignalBlocker blocker(cleanScreen);
                    cleanScreen->setCheckable(fullScreen->isCheckable());
                    cleanScreen->setChecked(fullScreen->isChecked());
                    cleanScreen->setEnabled(fullScreen->isEnabled());
                    cleanScreen->setIcon(QIcon(cleanIconPath));
                });
        registerStatusItem(cleanScreen, QStringLiteral("CleanScreen"),
                           tr("Clean Screen"), true);
    }

    // Match the control progression shown across reference pages 1-11 even
    // though several widgets are constructed later to reuse native helpers.
    const QStringList referenceStatusOrder {
        QStringLiteral("Coordinates"), QStringLiteral("ModelSpace"),
        QStringLiteral("Grid"), QStringLiteral("SnapMode"),
        QStringLiteral("InferConstraints"), QStringLiteral("DynamicInput"),
        QStringLiteral("OrthoMode"), QStringLiteral("SnapAngle"),
        QStringLiteral("IsometricDrafting"), QStringLiteral("ObjectSnap"),
        QStringLiteral("SnapTracking"), QStringLiteral("Lineweight"),
        QStringLiteral("QuickProperties"), QStringLiteral("CleanScreen")
    };
    for (const StatusItem& item : statusItems) {
        const int order = referenceStatusOrder.indexOf(item.key);
        if (order >= 0) item.widget->setProperty("kuubikStatusOrder", order);
    }

    auto* customizeButton = new QToolButton(statusBar());
    customizeButton->setObjectName(QStringLiteral("kuubikStatusCustomize"));
    customizeButton->setIcon(QIcon(QStringLiteral(":/icons/kuubik/view/status-customize.svg")));
    customizeButton->setIconSize(QSize(18, 18));
    customizeButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
    customizeButton->setPopupMode(QToolButton::InstantPopup);
    customizeButton->setAutoRaise(false);
    customizeButton->setToolTip(tr("Customization"));
    customizeButton->setAccessibleName(tr("Customize status bar"));

    auto* customizeMenu = new QMenu(customizeButton);
    customizeMenu->setObjectName(QStringLiteral("kuubikStatusCustomizationMenu"));
    for (const StatusItem& item : statusItems) {
        QAction* visibilityAction = customizeMenu->addAction(item.label);
        visibilityAction->setObjectName(
            QStringLiteral("kuubikStatusVisibility_%1").arg(item.key));
        visibilityAction->setProperty("kuubikStatusKey", item.key);
        visibilityAction->setCheckable(true);
        visibilityAction->setChecked(statusSettings.value(
            QStringLiteral("KuubikStatus/Visible/") + item.key,
            item.defaultVisible).toBool());
        connect(visibilityAction, &QAction::toggled, item.widget,
                [item](bool visible) {
                    QSettings().setValue(
                        QStringLiteral("KuubikStatus/Visible/") + item.key,
                        visible);
                    item.widget->setVisible(visible);
                });
    }
    customizeMenu->addSeparator();
    QAction* restoreDefaults = customizeMenu->addAction(tr("Reset Status Bar"));
    restoreDefaults->setObjectName(QStringLiteral("kuubikStatusReset"));
    connect(restoreDefaults, &QAction::triggered, this,
            [this, statusItems, customizeMenu]() {
                for (const StatusItem& item : statusItems) {
                    QSettings().setValue(
                        QStringLiteral("KuubikStatus/Visible/") + item.key,
                        item.defaultVisible);
                    if (QAction* action = customizeMenu->findChild<QAction*>(
                            QStringLiteral("kuubikStatusVisibility_%1").arg(item.key))) {
                        action->setChecked(item.defaultVisible);
                    }
                }
                applyKuubikStatusVisibility();
            });
    customizeButton->setMenu(customizeMenu);
    bar->addPermanentWidget(customizeButton);
    bar->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(bar, &QWidget::customContextMenuRequested, customizeMenu,
            [bar, customizeMenu](const QPoint& pos) {
                customizeMenu->popup(bar->mapToGlobal(pos));
            });

    configureKuubikCoordinateWidget(true);
    applyKuubikStatusVisibility();
}

void QC_ApplicationWindow::configureKuubikCoordinateWidget(bool kuubikMode)
{
    if (coordinateWidget == nullptr) return;

    if (!kuubikMode) {
        statusBar()->removeWidget(coordinateWidget);
        statusBar()->insertWidget(0, coordinateWidget);
        coordinateWidget->setProperty("kuubikCompactCoordinates", false);
        coordinateWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
        coordinateWidget->setMinimumSize(380, 27);
        coordinateWidget->setMaximumSize(1500, 160);
        coordinateWidget->lCoord1->show();
        coordinateWidget->lCoord1b->show();
        coordinateWidget->lCoord2->show();
        coordinateWidget->lCoord2b->show();
        coordinateWidget->line1->show();
        return;
    }

    applyKuubikStatusOrder();
    coordinateWidget->setProperty("kuubikCompactCoordinates", true);
    coordinateWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    coordinateWidget->setMinimumSize(184, 24);
    coordinateWidget->setMaximumSize(184, 24);
    coordinateWidget->line1->hide();
    if (coordinateWidget->lCoord1->text().count(QLatin1Char(',')) < 2) {
        coordinateWidget->lCoord1->setText(
            QStringLiteral("0.0000, 0.0000, 0.0000"));
    }
    if (coordinateWidget->lCoord2->text().count(QLatin1Char(',')) < 2) {
        coordinateWidget->lCoord2->setText(
            QStringLiteral("@ 0.0000, 0.0000, 0.0000"));
    }
    const QString mode = QSettings().value(
        QStringLiteral("KuubikStatus/CoordinateMode"),
        QStringLiteral("absolute-cartesian")).toString();
    coordinateWidget->lCoord1->setVisible(mode == QStringLiteral("absolute-cartesian"));
    coordinateWidget->lCoord2->setVisible(mode == QStringLiteral("relative-cartesian"));
    coordinateWidget->lCoord1b->setVisible(mode == QStringLiteral("absolute-polar"));
    coordinateWidget->lCoord2b->setVisible(mode == QStringLiteral("relative-polar"));
}

void QC_ApplicationWindow::applyKuubikStatusOrder()
{
    QStatusBar* bar = statusBar();
    QList<QWidget*> items;
    QHash<QWidget*, bool> visibility;
    const auto statusWidgets = bar->findChildren<QWidget*>();
    for (QWidget* widget : statusWidgets) {
        if (widget->property("kuubikStatusItem").toBool()) {
            items.append(widget);
            visibility.insert(widget, widget->isVisible());
        }
    }
    std::sort(items.begin(), items.end(), [](QWidget* left, QWidget* right) {
        return left->property("kuubikStatusOrder").toInt()
            < right->property("kuubikStatusOrder").toInt();
    });

    QToolButton* customize = bar->findChild<QToolButton*>(
        QStringLiteral("kuubikStatusCustomize"));
    for (QWidget* widget : items) {
        bar->removeWidget(widget);
    }
    if (customize != nullptr) {
        bar->removeWidget(customize);
    }
    for (QWidget* widget : items) {
        bar->addPermanentWidget(widget);
        widget->setVisible(visibility.value(widget));
    }
    if (customize != nullptr) {
        bar->addPermanentWidget(customize);
    }
}

void QC_ApplicationWindow::applyKuubikStatusVisibility()
{
    QSettings settings;
    const auto widgets = statusBar()->findChildren<QWidget*>();
    for (QWidget* widget : widgets) {
        if (!widget->property("kuubikStatusItem").toBool()) continue;
        const QString key = widget->property("kuubikStatusKey").toString();
        const bool defaultVisible = widget->property(
            "kuubikStatusDefaultVisible").toBool();
        widget->setVisible(settings.value(
            QStringLiteral("KuubikStatus/Visible/") + key,
            defaultVisible).toBool());
    }
    if (auto* customize = statusBar()->findChild<QToolButton*>(
            QStringLiteral("kuubikStatusCustomize"))) {
        customize->show();
    }
}

void QC_ApplicationWindow::initializeKuubikVisuals()
{
    QSettings settings;
    if (!qEnvironmentVariable("KUUBIK_UI_CONTRACT_PATH").isEmpty()) {
        settings.setValue("Workspace/Version", 2);
        settings.setValue("Workspace/Mode", "kuubik");
        settings.setValue("Workspace/PaletteSide", "right");
        applyKuubikWorkspace(true);
        return;
    }

    const int workspaceVersion = settings.value("Workspace/Version", 0).toInt();
    const QString workspaceMode = settings.value("Workspace/Mode", "kuubik").toString();

    if (workspaceVersion < 2) {
        settings.setValue("Appearance/Theme", "kuubik-dark");
        settings.setValue("Workspace/Version", 2);
        settings.setValue("Workspace/Mode", "kuubik");
        settings.setValue("Workspace/PaletteSide", "right");
        applyKuubikWorkspace(true);
    } else if (workspaceMode == "classic") {
        applyClassicWorkspace();
    } else {
        applyKuubikWorkspace(false);
    }
}

void QC_ApplicationWindow::applyKuubikTheme()
{
    KuubikTheme::apply();

    QSettings settings;
    settings.setValue("Appearance/Theme", "kuubik-dark");

    KuubikTheme::applyCanvasSettings();

    updateGrids();
    redrawAll();
}

void QC_ApplicationWindow::applyKuubikWorkspace(bool resetLayout)
{
    QSettings settings;
    settings.setValue("Workspace/Version", 2);
    settings.setValue("Workspace/Mode", "kuubik");
    settings.setValue("Startup/TabMode", 1);

    applyKuubikTheme();
    if (kuubikRibbon != nullptr) {
        kuubikRibbon->embedNativeToolbars(this);
    }
    if (kuubikRibbonToolbar != nullptr) {
        kuubikRibbonToolbar->show();
    }
    menuBar()->hide();

    mdiAreaCAD->setViewMode(QMdiArea::TabbedView);
    mdiAreaCAD->setTabsClosable(true);
    mdiAreaCAD->setTabsMovable(true);
    mdiAreaCAD->setTabPosition(QTabWidget::North);
    mdiAreaCAD->setTabShape(QTabWidget::Rounded);
    const auto tabBars = mdiAreaCAD->findChildren<QTabBar*>();
    for (auto* tabBar : tabBars) {
        tabBar->setExpanding(false);
    }

    statusBar()->setMinimumHeight(KuubikTheme::statusBarHeight());
    statusBar()->setMaximumHeight(KuubikTheme::statusBarHeight());
    if (snapToolBar != nullptr) snapToolBar->hide();
    const auto kuubikToolbars = findChildren<QToolBar*>(
        QString(), Qt::FindDirectChildrenOnly);
    for (QToolBar* toolbar : kuubikToolbars) {
        if (toolbar != kuubikRibbonToolbar) toolbar->hide();
    }
    configureKuubikCoordinateWidget(true);
    if (mouseWidget != nullptr) mouseWidget->hide();
    if (selectionWidget != nullptr) selectionWidget->hide();
    if (m_pActiveLayerName != nullptr) m_pActiveLayerName->hide();
    if (grid_status != nullptr) grid_status->hide();
    applyKuubikStatusVisibility();

    auto* commandDock = findChild<QDockWidget*>("command_dockwidget");
    if (commandDock != nullptr) {
        addDockWidget(Qt::BottomDockWidgetArea, commandDock);
        commandDock->setMinimumHeight(KuubikTheme::commandMinimumHeight());
        commandDock->setMaximumHeight(KuubikTheme::commandMaximumHeight());
        commandDock->show();
        modifyCommandTitleBar(Qt::BottomDockWidgetArea);
    }
    if (commandWidget != nullptr) {
        commandWidget->lCommand->hide();
        commandWidget->teHistory->setMinimumHeight(24);
        commandWidget->teHistory->setMaximumHeight(32);
    }

    if (resetLayout) {
        const QStringList primaryDocks {
            "kuubikPropertiesDock", "layer_dockwidget", "block_dockwidget",
            "command_dockwidget"
        };
        const auto docks = findChildren<QDockWidget*>();
        for (auto* dock : docks) {
            dock->setVisible(primaryDocks.contains(dock->objectName()));
        }

        const QString side = settings.value("Workspace/PaletteSide", "right").toString();
        setKuubikPaletteSide(side == "left" ? Qt::LeftDockWidgetArea
                                             : Qt::RightDockWidgetArea,
                             false);
    }
    if (kuubikPropertiesDock != nullptr) {
        const bool quickPropertiesVisible = settings.value(
            QStringLiteral("KuubikStatus/QuickProperties"), true).toBool();
        kuubikPropertiesDock->setVisible(quickPropertiesVisible);
    }

    if (kuubikWorkspaceAction != nullptr) {
        kuubikWorkspaceAction->setChecked(true);
    }
    if (classicWorkspaceAction != nullptr) {
        classicWorkspaceAction->setChecked(false);
    }
}

void QC_ApplicationWindow::applyClassicWorkspace()
{
    QSettings settings;
    settings.setValue("Workspace/Mode", "classic");

    qApp->setStyleSheet(QString());
    QApplication::setStyle(QStyleFactory::create("Fusion"));

    if (kuubikRibbon != nullptr) {
        kuubikRibbon->releaseNativeToolbars(this);
    }
    if (kuubikRibbonToolbar != nullptr) {
        kuubikRibbonToolbar->hide();
    }
    menuBar()->show();
    configureKuubikCoordinateWidget(false);
    if (coordinateWidget != nullptr) coordinateWidget->show();
    if (mouseWidget != nullptr) mouseWidget->show();
    if (selectionWidget != nullptr) selectionWidget->show();
    if (m_pActiveLayerName != nullptr) m_pActiveLayerName->show();
    if (grid_status != nullptr) grid_status->show();
    const auto kuubikStatusItems = statusBar()->findChildren<QWidget*>();
    for (QWidget* widget : kuubikStatusItems) {
        if (widget != coordinateWidget
            && widget->property("kuubikStatusItem").toBool()) {
            widget->hide();
        }
    }
    if (auto* customize = statusBar()->findChild<QToolButton*>(
            QStringLiteral("kuubikStatusCustomize"))) {
        customize->hide();
    }

    const QStringList classicToolbars {
        "file_toolbar", "edit_toolbar", "view_toolbar", "pen_toolbar",
        "options_toolbar", "snap_toolbar", "line_toolbar", "modify_toolbar"
    };
    const auto toolbars = findChildren<QToolBar*>();
    for (auto* toolbar : toolbars) {
        if (classicToolbars.contains(toolbar->objectName())) {
            toolbar->show();
        }
    }

    auto* commandDock = findChild<QDockWidget*>("command_dockwidget");
    if (commandDock != nullptr) {
        commandDock->setMinimumHeight(0);
        commandDock->setMaximumHeight(QWIDGETSIZE_MAX);
        modifyCommandTitleBar(dockWidgetArea(commandDock));
    }
    if (commandWidget != nullptr) {
        commandWidget->lCommand->show();
        commandWidget->teHistory->setMinimumHeight(46);
        commandWidget->teHistory->setMaximumHeight(QWIDGETSIZE_MAX);
    }

    if (classicWorkspaceAction != nullptr) {
        classicWorkspaceAction->setChecked(true);
    }
    if (kuubikWorkspaceAction != nullptr) {
        kuubikWorkspaceAction->setChecked(false);
    }
}

void QC_ApplicationWindow::setKuubikPaletteSide(Qt::DockWidgetArea area,
                                                bool persist)
{
    if (area != Qt::LeftDockWidgetArea && area != Qt::RightDockWidgetArea) {
        return;
    }

    auto* layerDock = findChild<QDockWidget*>("layer_dockwidget");
    auto* blockDock = findChild<QDockWidget*>("block_dockwidget");
    if (kuubikPropertiesDock == nullptr || layerDock == nullptr || blockDock == nullptr) {
        return;
    }

    removeDockWidget(kuubikPropertiesDock);
    removeDockWidget(layerDock);
    removeDockWidget(blockDock);
    addDockWidget(area, kuubikPropertiesDock);
    addDockWidget(area, layerDock);
    addDockWidget(area, blockDock);
    tabifyDockWidget(kuubikPropertiesDock, layerDock);
    tabifyDockWidget(kuubikPropertiesDock, blockDock);
    kuubikPropertiesDock->show();
    layerDock->show();
    blockDock->show();
    kuubikPropertiesDock->raise();
    resizeDocks({kuubikPropertiesDock}, {320}, Qt::Horizontal);

    const bool onLeft = area == Qt::LeftDockWidgetArea;
    if (paletteLeftAction != nullptr) {
        paletteLeftAction->setChecked(onLeft);
    }
    if (paletteRightAction != nullptr) {
        paletteRightAction->setChecked(!onLeft);
    }
    if (persist) {
        QSettings settings;
        settings.setValue("Workspace/PaletteSide", onLeft ? "left" : "right");
    }
}

bool QC_ApplicationWindow::writeKuubikUiContract(const QString& path)
{
    bool widthOk = false;
    bool heightOk = false;
    const int requestedWidth = qEnvironmentVariable("KUUBIK_UI_CONTRACT_WIDTH").toInt(&widthOk);
    const int requestedHeight = qEnvironmentVariable("KUUBIK_UI_CONTRACT_HEIGHT").toInt(&heightOk);
    if (widthOk && heightOk && requestedWidth > 0 && requestedHeight > 0) {
        setWindowState(windowState() & ~Qt::WindowMaximized);
        resize(requestedWidth, requestedHeight);
        show();
        QApplication::processEvents();
    }

    QJsonObject contract;
    QSettings settings;
    const QString previousWorkspaceMode = settings.value(
        "Workspace/Mode", QStringLiteral("kuubik")).toString();
    QHash<QToolBar*, bool> previousToolbarVisibility;
    const auto previousToolbars = findChildren<QToolBar*>();
    for (QToolBar* toolbar : previousToolbars) {
        previousToolbarVisibility.insert(toolbar, toolbar->isVisible());
    }
    const bool kuubikMenuBarVisible = menuBar()->isVisible();
    applyClassicWorkspace();
    QApplication::processEvents();
    const bool classicMenuBarVisible = menuBar()->isVisible();
    const bool classicCoordinateSlotRestored = coordinateWidget != nullptr
        && mouseWidget != nullptr
        && coordinateWidget->geometry().left()
            < mouseWidget->geometry().left();
    if (previousWorkspaceMode == QStringLiteral("classic")) {
        applyClassicWorkspace();
    } else {
        applyKuubikWorkspace(false);
        for (auto it = previousToolbarVisibility.cbegin();
             it != previousToolbarVisibility.cend(); ++it) {
            it.key()->setVisible(it.value());
        }
    }
    QApplication::processEvents();

    contract.insert("schemaVersion", 2);
    contract.insert("product", qApp->applicationName());
    contract.insert("version", qApp->applicationVersion());
    contract.insert("workspaceMode", settings.value("Workspace/Mode", "kuubik").toString());
    contract.insert("workspaceVersion", settings.value("Workspace/Version", 0).toInt());
    contract.insert("theme", settings.value("Appearance/Theme", "").toString());
    contract.insert("paletteSide", settings.value("Workspace/PaletteSide", "right").toString());
    contract.insert("ribbonVisible", kuubikRibbonToolbar != nullptr
                                      && kuubikRibbonToolbar->isVisible());
    contract.insert("menuBarVisible", kuubikMenuBarVisible);
    contract.insert("classicMenuBarVisible", classicMenuBarVisible);

    QJsonObject selectorObject;
    selectorObject.insert("present", kuubikCurrentLayerSelector != nullptr);
    selectorObject.insert("enabled", kuubikCurrentLayerSelector != nullptr
                                         && kuubikCurrentLayerSelector->isEnabled());
    const QString selectorLayer = kuubikCurrentLayerSelector == nullptr
                                      ? QString()
                                      : kuubikCurrentLayerSelector->currentText();
    const QString nativeLayer = kuubikCurrentLayerSelector == nullptr
                                    || kuubikCurrentLayerSelector->layerList() == nullptr
                                    || kuubikCurrentLayerSelector->layerList()->getActive() == nullptr
                                    ? QString()
                                    : kuubikCurrentLayerSelector->layerList()->getActive()->getName();
    selectorObject.insert("currentLayer", selectorLayer);
    selectorObject.insert("nativeCurrentLayer", nativeLayer);
    selectorObject.insert("width", kuubikCurrentLayerSelector == nullptr
                                       ? 0 : kuubikCurrentLayerSelector->width());
    selectorObject.insert("minimumWidth", kuubikCurrentLayerSelector == nullptr
                                              ? 0 : kuubikCurrentLayerSelector->minimumWidth());
    selectorObject.insert("embeddedInRibbon", kuubikRibbon != nullptr
                                               && kuubikCurrentLayerSelector != nullptr
                                               && kuubikRibbon->isAncestorOf(
                                                      kuubikCurrentLayerSelector));
    contract.insert("kuubikCurrentLayerSelector", selectorObject);

    QJsonArray bindings;
    QJsonArray mismatches;
    if (kuubikRibbon != nullptr) {
        contract.insert("boundActionKeys",
                        QJsonArray::fromStringList(kuubikRibbon->boundActionKeys()));
        contract.insert("missingActionKeys",
                        QJsonArray::fromStringList(kuubikRibbon->missingActionKeys()));
        for (const auto& key : kuubikRibbon->boundActionKeys()) {
            auto* button = kuubikRibbon->buttonForAction(key);
            QJsonObject binding;
            binding.insert("key", key);
            binding.insert("buttonActionKey", button == nullptr
                                                 ? QString()
                                                 : button->property("kuubikActionKey").toString());
            binding.insert("defaultActionObjectName",
                           button == nullptr || button->defaultAction() == nullptr
                               ? QString()
                               : button->defaultAction()->objectName());
            const bool valid = button != nullptr
                               && button->defaultAction() == a_map.value(key, nullptr)
                               && button->property("kuubikActionKey").toString() == key;
            binding.insert("valid", valid);
            bindings.append(binding);
            if (!valid) {
                mismatches.append(key);
            }
        }
    }
    contract.insert("bindings", bindings);
    contract.insert("bindingMismatches", mismatches);

    QJsonArray ribbonPanels;
    if (kuubikRibbon != nullptr) {
        const auto frames = kuubikRibbon->findChildren<QFrame*>(
            QStringLiteral("kuubikRibbonGroup"));
        for (QFrame* frame : frames) {
            const QString title = frame->property("kuubikPanelTitle").toString();
            if (title.isEmpty()) continue;
            QWidget* page = frame->parentWidget();
            const QString tab = page == nullptr
                                    ? QString()
                                    : page->property("kuubikTabTitle").toString();
            const QStringList actionKeys = frame->property("kuubikActionKeys").toStringList();
            bool identityValid = !tab.isEmpty() && !actionKeys.isEmpty();
            const auto buttons = frame->findChildren<QToolButton*>();
            for (const QString& key : actionKeys) {
                bool keyValid = false;
                for (QToolButton* panelButton : buttons) {
                    if (panelButton->property("kuubikActionKey").toString() == key
                        && panelButton->defaultAction() == a_map.value(key, nullptr)) {
                        keyValid = true;
                        break;
                    }
                }
                identityValid = identityValid && keyValid;
            }
            QJsonObject panelObject;
            panelObject.insert("tab", tab);
            panelObject.insert("title", title);
            panelObject.insert("collapsed", frame->property("kuubikCollapsed").toBool());
            panelObject.insert("actionKeys", QJsonArray::fromStringList(actionKeys));
            panelObject.insert("actionIdentityValid", identityValid);
            ribbonPanels.append(panelObject);
        }
    }
    contract.insert("ribbonPanels", ribbonPanels);

    QJsonArray visibleToolbars;
    const auto toolbars = findChildren<QToolBar*>();
    for (auto* toolbar : toolbars) {
        if (toolbar->isVisible()) {
            visibleToolbars.append(toolbar->objectName());
        }
    }
    contract.insert("visibleToolbars", visibleToolbars);

    auto areaName = [](Qt::DockWidgetArea area) {
        switch (area) {
        case Qt::LeftDockWidgetArea: return QStringLiteral("left");
        case Qt::RightDockWidgetArea: return QStringLiteral("right");
        case Qt::TopDockWidgetArea: return QStringLiteral("top");
        case Qt::BottomDockWidgetArea: return QStringLiteral("bottom");
        default: return QStringLiteral("floating");
        }
    };
    QJsonArray docks;
    const auto dockWidgets = findChildren<QDockWidget*>();
    for (auto* dock : dockWidgets) {
        QJsonObject dockObject;
        dockObject.insert("objectName", dock->objectName());
        dockObject.insert("visible", dock->isVisible());
        dockObject.insert("area", areaName(dockWidgetArea(dock)));
        docks.append(dockObject);
    }
    contract.insert("docks", docks);

    QJsonObject propertiesDockObject;
    propertiesDockObject.insert("present", kuubikPropertiesDock != nullptr);
    propertiesDockObject.insert("area", kuubikPropertiesDock == nullptr
                                             ? QStringLiteral("floating")
                                             : areaName(dockWidgetArea(kuubikPropertiesDock)));
    propertiesDockObject.insert("width", kuubikPropertiesDock == nullptr
                                           ? 0 : kuubikPropertiesDock->width());
    propertiesDockObject.insert("minimumWidth", kuubikPropertiesDock == nullptr
                                                  ? 0 : kuubikPropertiesDock->minimumWidth());
    auto* fullPropertiesButton = kuubikPropertiesPalette == nullptr
                                     ? nullptr
                                     : kuubikPropertiesPalette->findChild<QToolButton*>(
                                           QStringLiteral("kuubikOpenFullProperties"));
    propertiesDockObject.insert("modifyEntityNativeBinding",
                                fullPropertiesButton != nullptr
                                && fullPropertiesButton->defaultAction()
                                       == a_map.value("ModifyEntity", nullptr));
    QStringList tabGroupNames;
    QString activeTabObjectName;
    if (kuubikPropertiesDock != nullptr) {
        tabGroupNames.append(kuubikPropertiesDock->objectName());
        const auto tabified = tabifiedDockWidgets(kuubikPropertiesDock);
        for (QDockWidget* dock : tabified) tabGroupNames.append(dock->objectName());
        tabGroupNames.removeDuplicates();
        tabGroupNames.sort();

        const auto normalizedTabTitle = [](QString title) {
            title.remove(QLatin1Char('&'));
            return title.simplified();
        };
        QHash<QString, QDockWidget*> docksByTitle;
        for (const QString& objectName : tabGroupNames) {
            if (QDockWidget* dock = findChild<QDockWidget*>(objectName)) {
                docksByTitle.insert(normalizedTabTitle(dock->windowTitle()), dock);
            }
        }
        const auto dockTabBars = findChildren<QTabBar*>();
        for (QTabBar* tabBar : dockTabBars) {
            QSet<QString> matchingTitles;
            for (int index = 0; index < tabBar->count(); ++index) {
                const QString title = normalizedTabTitle(tabBar->tabText(index));
                if (docksByTitle.contains(title)) {
                    matchingTitles.insert(title);
                }
            }
            if (matchingTitles.size() != docksByTitle.size()
                || tabBar->currentIndex() < 0) {
                continue;
            }

            const QString activeTitle = normalizedTabTitle(
                tabBar->tabText(tabBar->currentIndex()));
            if (QDockWidget* activeDock = docksByTitle.value(activeTitle, nullptr)) {
                activeTabObjectName = activeDock->objectName();
            }
            break;
        }
    }
    propertiesDockObject.insert("tabGroupObjectNames",
                                QJsonArray::fromStringList(tabGroupNames));
    propertiesDockObject.insert("activeTabObjectName", activeTabObjectName);
    contract.insert("kuubikPropertiesDock", propertiesDockObject);

    QJsonObject classicSampleAction;
    const QString classicSampleKey = QStringLiteral("DrawLineBisector");
    QAction* classicSample = a_map.value(classicSampleKey, nullptr);
    bool classicSampleInMenu = false;
    if (classicSample != nullptr) {
        for (QWidget* associated : classicSample->associatedWidgets()) {
            if (qobject_cast<QMenu*>(associated) != nullptr) {
                classicSampleInMenu = true;
                break;
            }
        }
    }
    classicSampleAction.insert("key", classicSampleKey);
    classicSampleAction.insert("presentInMenu", classicSampleInMenu);
    contract.insert("classicSampleAction", classicSampleAction);

    QJsonObject commandLineObject;
    auto* commandDock = findChild<QDockWidget*>(QStringLiteral("command_dockwidget"));
    commandLineObject.insert("present", commandWidget != nullptr && commandDock != nullptr);
    commandLineObject.insert("nativeBinding", commandWidget != nullptr
                                              && commandDock != nullptr
                                              && commandDock->widget() == commandWidget);
    contract.insert("commandLine", commandLineObject);

    QJsonArray statusControls;
    bool customStatusIconsStableAfterClick = true;
    int customStatusIconClickTests = 0;
    const auto statusButtons = statusBar()->findChildren<QToolButton*>();
    for (QToolButton* statusButton : statusButtons) {
        if (!statusButton->property("kuubikStatusItem").toBool()) continue;
        const QString key = statusButton->property("kuubikActionKey").toString();
        const QString bindingType = statusButton->property(
            "kuubikBindingType").toString();
        bool functionalBinding = false;
        if (bindingType == QStringLiteral("direct-action")) {
            functionalBinding = a_map.value(key, nullptr) != nullptr
                                && statusButton->property(
                                       "kuubikNativeActionBound").toBool()
                                && statusButton->defaultAction() == nullptr
                                && !statusButton->property(
                                        "kuubikIconPath").toString().isEmpty();
        } else if (bindingType == QStringLiteral("inverse-action")) {
            functionalBinding = a_map.value(key, nullptr) != nullptr;
        } else if (bindingType == QStringLiteral("snap-mode")) {
            functionalBinding = actionHandler != nullptr
                                && statusButton->property(
                                       "kuubikSnapMask").toUInt() != 0;
        } else if (bindingType == QStringLiteral("application-setting")) {
            functionalBinding = key == QStringLiteral("DynamicInput");
        } else if (bindingType == QStringLiteral("dock-visibility")) {
            functionalBinding = key == QStringLiteral("QuickProperties")
                                && kuubikPropertiesDock != nullptr;
        } else if (bindingType == QStringLiteral("object-snap-menu")) {
            functionalBinding = actionHandler != nullptr
                                && statusButton->menu() != nullptr
                                && statusButton->menu()->actions().size() >= 10;
        } else if (bindingType == QStringLiteral("inference-bundle")) {
            functionalBinding = actionHandler != nullptr
                                && statusButton->property(
                                       "kuubikNonParametric").toBool();
        } else if (bindingType == QStringLiteral("document-setting")) {
            QC_MDIWindow* mdi = getMDIWindow();
            functionalBinding = mdi != nullptr && mdi->getGraphic() != nullptr
                                && mdi->getGraphicView() != nullptr
                                && statusButton->menu() != nullptr;
        }
        QJsonObject statusObject;
        statusObject.insert("objectName", statusButton->objectName());
        statusObject.insert("actionKey", key);
        statusObject.insert("bindingType", bindingType);
        statusObject.insert("visibilityKey", statusButton->property(
            "kuubikStatusKey").toString());
        statusObject.insert("nativeBinding", !key.isEmpty() && functionalBinding);
        if (bindingType == QStringLiteral("direct-action")) {
            const QString iconPath = statusButton->property(
                "kuubikIconPath").toString();
            const QIcon ownedIcon(iconPath);
            const QSize sampleSize = statusButton->iconSize();
            const bool customIconOwned = statusButton->defaultAction() == nullptr
                && !iconPath.isEmpty() && !ownedIcon.isNull()
                && statusButton->icon().pixmap(sampleSize).toImage()
                    == ownedIcon.pixmap(sampleSize).toImage();
            statusObject.insert("customIconOwned", customIconOwned);

            QAction* nativeAction = a_map.value(key, nullptr);
            if (nativeAction != nullptr && nativeAction->isCheckable()
                && key != QStringLiteral("Fullscreen")) {
                const bool checkedBefore = nativeAction->isChecked();
                statusButton->click();
                QApplication::processEvents();
                const bool stableAfterClick =
                    statusButton->icon().pixmap(sampleSize).toImage()
                    == ownedIcon.pixmap(sampleSize).toImage();
                statusButton->click();
                QApplication::processEvents();
                const bool stateRestored =
                    nativeAction->isChecked() == checkedBefore;
                statusObject.insert("customIconStableAfterClick",
                                    stableAfterClick);
                statusObject.insert("nativeStateRestoredAfterClickTest",
                                    stateRestored);
                customStatusIconsStableAfterClick =
                    customStatusIconsStableAfterClick && customIconOwned
                    && stableAfterClick && stateRestored;
                ++customStatusIconClickTests;
            }
        }
        statusControls.append(statusObject);
    }
    contract.insert("statusControls", statusControls);

    QJsonObject statusBarObject;
    statusBarObject.insert("customIconClickTests",
                           customStatusIconClickTests);
    statusBarObject.insert("customIconsStableAfterClick",
                           customStatusIconsStableAfterClick
                           && customStatusIconClickTests >= 3);
    auto* customizeButton = statusBar()->findChild<QToolButton*>(
        QStringLiteral("kuubikStatusCustomize"));
    auto* customizeMenu = statusBar()->findChild<QMenu*>(
        QStringLiteral("kuubikStatusCustomizationMenu"));
    QJsonArray customizationEntries;
    if (customizeMenu != nullptr) {
        for (QAction* action : customizeMenu->actions()) {
            const QString visibilityKey = action->property(
                "kuubikStatusKey").toString();
            if (visibilityKey.isEmpty()) continue;
            QJsonObject entry;
            entry.insert("key", visibilityKey);
            entry.insert("checked", action->isChecked());
            bool controlPresent = false;
            const auto widgets = statusBar()->findChildren<QWidget*>();
            for (QWidget* widget : widgets) {
                if (widget->property("kuubikStatusKey").toString()
                    == visibilityKey) {
                    controlPresent = true;
                    break;
                }
            }
            entry.insert("controlPresent", controlPresent);
            customizationEntries.append(entry);
        }
    }

    bool customizationToggleRoundTrip = false;
    if (customizeMenu != nullptr) {
        QAction* gridVisibility = customizeMenu->findChild<QAction*>(
            QStringLiteral("kuubikStatusVisibility_Grid"));
        QWidget* gridControl = nullptr;
        const auto widgets = statusBar()->findChildren<QWidget*>();
        for (QWidget* widget : widgets) {
            if (widget->property("kuubikStatusKey").toString()
                == QStringLiteral("Grid")) {
                gridControl = widget;
                break;
            }
        }
        if (gridVisibility != nullptr && gridControl != nullptr) {
            const bool checkedBefore = gridVisibility->isChecked();
            const bool hiddenBefore = gridControl->isHidden();
            gridVisibility->trigger();
            QApplication::processEvents();
            const bool toggled = gridVisibility->isChecked() != checkedBefore
                                 && gridControl->isHidden() != hiddenBefore;
            gridVisibility->trigger();
            QApplication::processEvents();
            customizationToggleRoundTrip = toggled
                && gridVisibility->isChecked() == checkedBefore
                && gridControl->isHidden() == hiddenBefore;
        }
    }

    auto* coordinateMenu = coordinateWidget == nullptr
        ? nullptr
        : coordinateWidget->findChild<QMenu*>(
              QStringLiteral("kuubikCoordinateMenu"));
    auto* osnapStatusButton = statusBar()->findChild<QToolButton*>(
        QStringLiteral("kuubikOsnapButton"));
    int osnapModeCount = 0;
    if (osnapStatusButton != nullptr && osnapStatusButton->menu() != nullptr) {
        for (QAction* action : osnapStatusButton->menu()->actions()) {
            if (action->property("kuubikSnapMask").isValid()) ++osnapModeCount;
        }
    }
    const QString statusMenuScreenshotPath = qEnvironmentVariable(
        "KUUBIK_STATUS_MENU_SCREENSHOT_PATH");
    bool statusMenuScreenshotSaved = statusMenuScreenshotPath.isEmpty();
    int statusMenuScreenshotWidth = 0;
    int statusMenuScreenshotHeight = 0;
    if (!statusMenuScreenshotPath.isEmpty() && customizeMenu != nullptr) {
        customizeMenu->ensurePolished();
        customizeMenu->adjustSize();
        customizeMenu->show();
        QApplication::processEvents();
        const QPixmap menuScreenshot = customizeMenu->grab();
        statusMenuScreenshotWidth = menuScreenshot.width();
        statusMenuScreenshotHeight = menuScreenshot.height();
        statusMenuScreenshotSaved = menuScreenshot.save(
            statusMenuScreenshotPath, "PNG");
        customizeMenu->hide();
    }
    statusBarObject.insert("customizationButtonPresent", customizeButton != nullptr);
    statusBarObject.insert("customizationMenuPresent", customizeMenu != nullptr);
    statusBarObject.insert("customizationEntries", customizationEntries);
    statusBarObject.insert("customizationToggleRoundTrip",
                           customizationToggleRoundTrip);
    statusBarObject.insert("coordinateDisplayPresent", coordinateWidget != nullptr);
    statusBarObject.insert("coordinateModeCount",
                           coordinateMenu == nullptr
                               ? 0 : coordinateMenu->actions().size());
    statusBarObject.insert("osnapModeCount", osnapModeCount);
    statusBarObject.insert("sizeGripDisabled", !statusBar()->isSizeGripEnabled());
    statusBarObject.insert("menuScreenshotSaved", statusMenuScreenshotSaved);
    statusBarObject.insert("menuScreenshotWidth", statusMenuScreenshotWidth);
    statusBarObject.insert("menuScreenshotHeight", statusMenuScreenshotHeight);

    QJsonObject referenceFirstFive;
    auto* modelStatus = statusBar()->findChild<QLabel*>(
        QStringLiteral("kuubikModelStatus"));
    QToolButton* gridStatusButton = nullptr;
    for (QToolButton* statusButton : statusButtons) {
        if (statusButton->property("kuubikActionKey").toString()
            == QStringLiteral("ViewGrid")) {
            gridStatusButton = statusButton;
            break;
        }
    }
    const bool coordinateVisible = coordinateWidget != nullptr
        && !coordinateWidget->isHidden();
    const bool modelVisible = modelStatus != nullptr && !modelStatus->isHidden();
    const bool gridVisible = gridStatusButton != nullptr
        && !gridStatusButton->isHidden();
    const bool orderedCluster = coordinateVisible && modelVisible && gridVisible
        && coordinateWidget->geometry().left() < modelStatus->geometry().left()
        && modelStatus->geometry().left()
            < gridStatusButton->geometry().left();
    const bool sameRow = coordinateVisible && modelVisible && gridVisible
        && qAbs(coordinateWidget->geometry().center().y()
                - modelStatus->geometry().center().y()) <= 2
        && qAbs(modelStatus->geometry().center().y()
                - gridStatusButton->geometry().center().y()) <= 2;
    QAction* gridAction = a_map.value(QStringLiteral("ViewGrid"), nullptr);
    QMenu* gridSettingsMenu = gridStatusButton == nullptr
        ? nullptr
        : gridStatusButton->findChild<QMenu*>(
              QStringLiteral("kuubikStatusSettingsMenu"));
    referenceFirstFive.insert("referencePageCount", 5);
    referenceFirstFive.insert("coordinatesVisibleByDefault", coordinateVisible);
    referenceFirstFive.insert("coordinateCompactWidth",
                              coordinateWidget == nullptr
                                  ? 0 : coordinateWidget->width());
    referenceFirstFive.insert("coordinateDisplaysZ",
                              coordinateWidget != nullptr
                              && coordinateWidget->lCoord1->text().count(
                                     QLatin1Char(',')) >= 2);
    referenceFirstFive.insert("coordinateModelGridOrdered", orderedCluster);
    referenceFirstFive.insert("coordinateModelGridSingleRow", sameRow);
    referenceFirstFive.insert("classicCoordinateSlotRestored",
                              classicCoordinateSlotRestored);
    referenceFirstFive.insert("modelIndicatorText",
                              modelStatus == nullptr
                                  ? QString() : modelStatus->text());
    referenceFirstFive.insert("gridSettingsMenuPresent",
                              gridSettingsMenu != nullptr
                              && !gridSettingsMenu->actions().isEmpty());
    referenceFirstFive.insert("gridStateSynchronized",
                              gridStatusButton != nullptr
                              && gridAction != nullptr
                              && gridStatusButton->isChecked()
                                  == gridAction->isChecked());
    referenceFirstFive.insert("gridTooltipMatchesReference",
                              gridStatusButton != nullptr
                              && gridStatusButton->toolTip().contains(
                                  QStringLiteral("F7"))
                              && gridStatusButton->toolTip().contains(
                                  QStringLiteral("Grid Settings")));
    referenceFirstFive.insert("statusBarHeight", statusBar()->height());
    statusBarObject.insert("referencePdfFirstFive", referenceFirstFive);

    const auto statusButtonForKey = [&statusButtons](const QString& key) {
        for (QToolButton* button : statusButtons) {
            if (button->property("kuubikActionKey").toString() == key) {
                return button;
            }
        }
        return static_cast<QToolButton*>(nullptr);
    };
    const auto actionHasShortcut = [this](const QString& key,
                                           const QKeySequence& shortcut) {
        QAction* action = a_map.value(key, nullptr);
        return action != nullptr && action->shortcuts().contains(shortcut);
    };
    QToolButton* snapStatusButton = statusButtonForKey(
        QStringLiteral("SnapGrid"));
    QToolButton* inferStatusButton = statusButtonForKey(
        QStringLiteral("InferConstraints"));
    QToolButton* dynamicStatusButton = statusButtonForKey(
        QStringLiteral("DynamicInput"));
    QToolButton* orthoStatusButton = statusButtonForKey(
        QStringLiteral("RestrictOrthogonal"));
    QToolButton* polarStatusButton = statusButtonForKey(
        QStringLiteral("SnapAngle"));
    QToolButton* isometricStatusButton = statusButtonForKey(
        QStringLiteral("IsometricDrafting"));
    QToolButton* trackingStatusButton = statusButtonForKey(
        QStringLiteral("SnapTracking"));
    QMenu* snapStatusMenu = snapStatusButton == nullptr
        ? nullptr : snapStatusButton->findChild<QMenu*>(
              QStringLiteral("kuubikSnapModeMenu"));
    QMenu* dynamicSettingsMenu = dynamicStatusButton == nullptr
        ? nullptr : dynamicStatusButton->findChild<QMenu*>(
              QStringLiteral("kuubikDynamicInputMenu"));
    QMenu* polarStatusMenu = polarStatusButton == nullptr
        ? nullptr : polarStatusButton->findChild<QMenu*>(
              QStringLiteral("kuubikPolarMenu"));
    QMenu* isometricStatusMenu = isometricStatusButton == nullptr
        ? nullptr : isometricStatusButton->findChild<QMenu*>(
              QStringLiteral("kuubikIsometricMenu"));

    int snapModeChoiceCount = 0;
    if (snapStatusMenu != nullptr) {
        for (QAction* action : snapStatusMenu->actions()) {
            if (action->objectName() == QStringLiteral("kuubikSnapModeGrid")
                || action->objectName()
                       == QStringLiteral("kuubikSnapModePolar")) {
                ++snapModeChoiceCount;
            }
        }
    }
    int polarPresetCount = 0;
    if (polarStatusMenu != nullptr) {
        for (QAction* action : polarStatusMenu->actions()) {
            if (action->property("kuubikPolarAngle").isValid()) {
                ++polarPresetCount;
            }
        }
    }
    int isoplaneCount = 0;
    if (isometricStatusMenu != nullptr) {
        for (QAction* action : isometricStatusMenu->actions()) {
            if (action->property("kuubikIsoplane").isValid()) {
                ++isoplaneCount;
            }
        }
    }

    bool inferenceBundleRoundTrip = false;
    if (inferStatusButton != nullptr && actionHandler != nullptr) {
        const RS_SnapMode original = actionHandler->getSnaps();
        {
            const QSignalBlocker blocker(inferStatusButton);
            inferStatusButton->setChecked(false);
        }
        inferStatusButton->click();
        QApplication::processEvents();
        const RS_SnapMode inferred = actionHandler->getSnaps();
        const bool enabledNativeBundle = inferred.snapEndpoint
            && inferred.snapPerpendicular && inferred.snapTangent
            && inferred.snapParallel;
        inferStatusButton->click();
        QApplication::processEvents();
        inferenceBundleRoundTrip = enabledNativeBundle
            && RS_SnapMode::toInt(actionHandler->getSnaps())
                   == RS_SnapMode::toInt(original);
        actionHandler->slotSetSnaps(original);
        {
            const QSignalBlocker blocker(inferStatusButton);
            inferStatusButton->setChecked(original.snapEndpoint
                && original.snapPerpendicular && original.snapTangent
                && original.snapParallel);
        }
    }

    bool polarSnapEngineQuantizes = false;
    QC_MDIWindow* referenceMdi = getMDIWindow();
    RS_Graphic* referenceGraphic = referenceMdi == nullptr
        ? nullptr : referenceMdi->getGraphic();
    QG_GraphicView* referenceView = referenceMdi == nullptr
        ? nullptr : referenceMdi->getGraphicView();
    QString originalPolarIncrement;
    {
        auto settingsGuard = RS_SETTINGS->beginGroupGuard("/Snap");
        originalPolarIncrement = RS_SETTINGS->readEntry(
            "/AngleIncrement", "15");
        RS_SETTINGS->writeEntry("/AngleIncrement", QStringLiteral("15"));
    }
    if (referenceGraphic != nullptr && referenceView != nullptr
        && referenceView->width() > 100 && referenceView->height() > 100) {
        RS_Snapper verifier(*referenceGraphic, *referenceView);
        verifier.init();
        RS_SnapMode polarOnly;
        polarOnly.snapAngle = true;
        verifier.setSnapMode(polarOnly);
        const QPoint testPoint(referenceView->width() * 7 / 10,
                               referenceView->height() * 2 / 5);
        const RS_Vector raw = referenceView->toGraph(testPoint.x(),
                                                      testPoint.y());
        QMouseEvent polarEvent(QEvent::MouseMove, QPointF(testPoint),
                               Qt::NoButton, Qt::NoButton, Qt::NoModifier);
        const RS_Vector snapped = verifier.snapPoint(&polarEvent);
        const RS_Vector relativeZero = referenceView->getRelativeZero();
        const double degrees = relativeZero.angleTo(snapped)
                               * 180.0 / M_PI;
        polarSnapEngineQuantizes = snapped.valid
            && raw.distanceTo(snapped) > RS_TOLERANCE
            && std::abs(std::remainder(degrees, 15.0)) < 0.001;
        verifier.finish();
    }
    {
        auto settingsGuard = RS_SETTINGS->beginGroupGuard("/Snap");
        RS_SETTINGS->writeEntry("/AngleIncrement", originalPolarIncrement);
    }

    bool isometricPlanesRoundTrip = false;
    if (referenceGraphic != nullptr && referenceView != nullptr
        && isometricStatusMenu != nullptr) {
        const bool originalIsometric = referenceGraphic->isIsometricGrid();
        const RS2::CrosshairType originalPlane =
            referenceGraphic->getCrosshairType();
        bool leftApplied = false;
        bool topApplied = false;
        bool rightApplied = false;
        for (QAction* action : isometricStatusMenu->actions()) {
            if (!action->property("kuubikIsoplane").isValid()) continue;
            const auto plane = static_cast<RS2::CrosshairType>(
                action->property("kuubikIsoplane").toInt());
            action->trigger();
            QApplication::processEvents();
            const bool applied = referenceGraphic->isIsometricGrid()
                && referenceGraphic->getCrosshairType() == plane
                && referenceView->getCrosshairType() == plane;
            if (plane == RS2::LeftCrosshair) leftApplied = applied;
            if (plane == RS2::TopCrosshair) topApplied = applied;
            if (plane == RS2::RightCrosshair) rightApplied = applied;
        }
        referenceGraphic->setIsometricGrid(originalIsometric);
        referenceGraphic->setCrosshairType(originalPlane);
        referenceView->setCrosshairType(originalPlane);
        referenceView->redraw(RS2::RedrawAll);
        {
            const QSignalBlocker blocker(isometricStatusButton);
            isometricStatusButton->setChecked(originalIsometric);
        }
        isometricPlanesRoundTrip = leftApplied && topApplied && rightApplied;
    }

    const QList<QToolButton*> pageSixToElevenButtons {
        snapStatusButton, inferStatusButton, dynamicStatusButton,
        orthoStatusButton, polarStatusButton, isometricStatusButton
    };
    bool pageControlsVisible = true;
    bool pageControlsOrdered = true;
    int priorLeft = -1;
    for (QToolButton* button : pageSixToElevenButtons) {
        const bool visible = button != nullptr && !button->isHidden();
        pageControlsVisible = pageControlsVisible && visible;
        if (visible) {
            pageControlsOrdered = pageControlsOrdered
                && button->geometry().left() > priorLeft;
            priorLeft = button->geometry().left();
        } else {
            pageControlsOrdered = false;
        }
    }

    QJsonObject referenceSixToEleven;
    referenceSixToEleven.insert("referenceStartPage", 6);
    referenceSixToEleven.insert("referenceEndPage", 11);
    referenceSixToEleven.insert("referencePageCount", 6);
    referenceSixToEleven.insert("pageControlsVisible", pageControlsVisible);
    referenceSixToEleven.insert("pageControlsOrdered", pageControlsOrdered);
    referenceSixToEleven.insert("snapModeChoiceCount", snapModeChoiceCount);
    referenceSixToEleven.insert("snapSettingsOnRightClick",
        snapStatusButton != nullptr
        && snapStatusButton->contextMenuPolicy() == Qt::CustomContextMenu);
    referenceSixToEleven.insert("snapF9Shortcut",
        actionHasShortcut(QStringLiteral("SnapGrid"),
                          QKeySequence(Qt::Key_F9)));
    referenceSixToEleven.insert("inferenceUsesNativeSnapBundle",
                                inferenceBundleRoundTrip);
    referenceSixToEleven.insert("inferenceIsHonestlyNonPersistent",
        inferStatusButton != nullptr
        && inferStatusButton->property("kuubikNonParametric").toBool()
        && inferStatusButton->toolTip().contains(
            QStringLiteral("not persistent")));
    referenceSixToEleven.insert("dynamicF12Shortcut",
        findChild<QShortcut*>(QStringLiteral("kuubikShortcutF12")) != nullptr);
    referenceSixToEleven.insert("dynamicSettingsPresent",
        dynamicSettingsMenu != nullptr
        && dynamicSettingsMenu->findChild<QAction*>(
            QStringLiteral("kuubikDynamicShowDistance")) != nullptr
        && dynamicSettingsMenu->findChild<QAction*>(
            QStringLiteral("kuubikDynamicShowAngle")) != nullptr);
    referenceSixToEleven.insert("orthoNativeBinding",
        orthoStatusButton != nullptr
        && orthoStatusButton->property("kuubikNativeActionBound").toBool());
    referenceSixToEleven.insert("orthoF8Shortcut",
        actionHasShortcut(QStringLiteral("RestrictOrthogonal"),
                          QKeySequence(Qt::Key_F8)));
    referenceSixToEleven.insert("polarPresetCount", polarPresetCount);
    referenceSixToEleven.insert("polarF10Shortcut",
        findChild<QShortcut*>(QStringLiteral("kuubikShortcutF10")) != nullptr);
    referenceSixToEleven.insert("polarSnapEngineQuantizes",
                                polarSnapEngineQuantizes);
    referenceSixToEleven.insert("isoplaneCount", isoplaneCount);
    referenceSixToEleven.insert("isometricPlanesRoundTrip",
                                isometricPlanesRoundTrip);
    referenceSixToEleven.insert("isometricF5Shortcut",
        findChild<QShortcut*>(QStringLiteral("kuubikShortcutF5")) != nullptr);
    referenceSixToEleven.insert("isometricCtrlEShortcut",
        findChild<QShortcut*>(QStringLiteral("kuubikShortcutCtrlE")) != nullptr);
    statusBarObject.insert("referencePdfPagesSixToEleven",
                           referenceSixToEleven);

    constexpr unsigned objectSnapMask = RS_SnapMode::SnapEndpoint
        | RS_SnapMode::SnapMiddle | RS_SnapMode::SnapCenter
        | RS_SnapMode::SnapIntersection | RS_SnapMode::SnapOnEntity
        | RS_SnapMode::SnapDistance | RS_SnapMode::SnapQuadrant
        | RS_SnapMode::SnapNode | RS_SnapMode::SnapInsertion
        | RS_SnapMode::SnapPerpendicular | RS_SnapMode::SnapTangent
        | RS_SnapMode::SnapGeometricCenter
        | RS_SnapMode::SnapApparentIntersection
        | RS_SnapMode::SnapExtension | RS_SnapMode::SnapParallel;
    const auto setObjectSnapBits = [](RS_SnapMode& mode, unsigned bits) {
        const RS_SnapMode source = RS_SnapMode::fromInt(bits);
        mode.snapEndpoint = source.snapEndpoint;
        mode.snapMiddle = source.snapMiddle;
        mode.snapCenter = source.snapCenter;
        mode.snapIntersection = source.snapIntersection;
        mode.snapOnEntity = source.snapOnEntity;
        mode.snapDistance = source.snapDistance;
        mode.snapQuadrant = source.snapQuadrant;
        mode.snapNode = source.snapNode;
        mode.snapInsertion = source.snapInsertion;
        mode.snapPerpendicular = source.snapPerpendicular;
        mode.snapTangent = source.snapTangent;
        mode.snapGeometricCenter = source.snapGeometricCenter;
        mode.snapApparentIntersection = source.snapApparentIntersection;
        mode.snapExtension = source.snapExtension;
        mode.snapParallel = source.snapParallel;
    };
    auto* osnapF3 = findChild<QShortcut*>(
        QStringLiteral("kuubikShortcutF3"));
    auto* trackingF11 = findChild<QShortcut*>(
        QStringLiteral("kuubikShortcutF11"));
    QAction* fullscreenAction = a_map.value(
        QStringLiteral("Fullscreen"), nullptr);
    const bool osnapF3Registered = osnapF3 != nullptr
        && osnapF3->key() == QKeySequence(Qt::Key_F3)
        && osnapF3->context() == Qt::ApplicationShortcut;
    const bool trackingF11Registered = trackingF11 != nullptr
        && trackingF11->key() == QKeySequence(Qt::Key_F11)
        && trackingF11->context() == Qt::ApplicationShortcut;
    bool fullscreenF11Removed = fullscreenAction != nullptr;
    if (fullscreenAction != nullptr) {
        for (const QKeySequence& shortcut : fullscreenAction->shortcuts()) {
            fullscreenF11Removed = fullscreenF11Removed
                && shortcut != QKeySequence(Qt::Key_F11);
        }
    }

    bool osnapStateRoundTrip = false;
    bool osnapMouseKeyboardSynchronized = false;
    bool osnapPriorSetPersisted = false;
    bool osnapNonObjectBitsPreserved = false;
    bool f3ActualKeyEvents = false;
    bool trackingStateRoundTrip = false;
    bool trackingMouseKeyboardSynchronized = false;
    bool trackingSerializationRoundTrip = false;
    bool f11ActualKeyEvents = false;
    if (actionHandler != nullptr && osnapStatusButton != nullptr
        && trackingStatusButton != nullptr) {
        const RS_SnapMode original = actionHandler->getSnaps();
        const QVariant originalSavedProperty = osnapStatusButton->property(
            "kuubikSavedOsnap");
        const QString savedOsnapKey = QStringLiteral(
            "KuubikStatus/SavedObjectSnapMode");
        const bool hadSavedOsnap = settings.contains(savedOsnapKey);
        const QVariant originalSavedOsnap = settings.value(savedOsnapKey);

        RS_SnapMode configured = original;
        setObjectSnapBits(configured, RS_SnapMode::SnapEndpoint
            | RS_SnapMode::SnapMiddle | RS_SnapMode::SnapCenter
            | RS_SnapMode::SnapIntersection | RS_SnapMode::SnapPerpendicular
            | RS_SnapMode::SnapTangent | RS_SnapMode::SnapOnEntity
            | RS_SnapMode::SnapExtension | RS_SnapMode::SnapParallel);
        configured.snapFree = true;
        configured.snapGrid = true;
        configured.snapAngle = true;
        configured.snapTracking = true;
        configured.restriction = RS2::RestrictVertical;
        const unsigned configuredBits = RS_SnapMode::toInt(configured);
        const unsigned configuredNonObjectBits = configuredBits
            & ~objectSnapMask;
        const auto preservesNonObjectBits = [configuredNonObjectBits](
                const RS_SnapMode& mode) {
            return (RS_SnapMode::toInt(mode) & ~objectSnapMask)
                == configuredNonObjectBits;
        };
        const auto sendFunctionKey = [this](Qt::Key key) {
            QWidget* target = QApplication::focusWidget();
            if (target == nullptr || target->window() != this) {
                target = this;
                target->setFocus(Qt::OtherFocusReason);
            }
            QKeyEvent press(QEvent::KeyPress, key, Qt::NoModifier);
            const bool pressSent = QApplication::sendEvent(target, &press);
            QKeyEvent release(QEvent::KeyRelease, key, Qt::NoModifier);
            const bool releaseSent = QApplication::sendEvent(target, &release);
            QApplication::processEvents();
            return pressSent && releaseSent;
        };
        actionHandler->slotSetSnaps(configured);
        {
            const QSignalBlocker blocker(osnapStatusButton);
            osnapStatusButton->setChecked(true);
        }

        osnapStatusButton->click();
        QApplication::processEvents();
        const RS_SnapMode disabled = actionHandler->getSnaps();
        const bool mouseDisabled =
            (RS_SnapMode::toInt(disabled) & objectSnapMask) == 0
            && preservesNonObjectBits(disabled)
            && !osnapStatusButton->isChecked();
        const unsigned persistedObjectBits = settings.value(
            savedOsnapKey, 0).toUInt() & objectSnapMask;

        const bool f3Invoked = osnapF3Registered
            && sendFunctionKey(Qt::Key_F3);
        const bool keyboardRestored = f3Invoked
            && RS_SnapMode::toInt(actionHandler->getSnaps()) == configuredBits
            && osnapStatusButton->isChecked();
        const bool f3DisabledAgain = osnapF3Registered
            && sendFunctionKey(Qt::Key_F3);
        const RS_SnapMode keyboardDisabledMode = actionHandler->getSnaps();
        const bool keyboardDisabled = f3DisabledAgain
            && (RS_SnapMode::toInt(keyboardDisabledMode) & objectSnapMask) == 0
            && preservesNonObjectBits(keyboardDisabledMode)
            && !osnapStatusButton->isChecked();
        osnapStatusButton->click();
        QApplication::processEvents();
        const bool mouseRestored =
            RS_SnapMode::toInt(actionHandler->getSnaps()) == configuredBits
            && osnapStatusButton->isChecked();
        osnapNonObjectBitsPreserved = preservesNonObjectBits(disabled)
            && preservesNonObjectBits(keyboardDisabledMode)
            && mouseRestored && keyboardRestored;
        f3ActualKeyEvents = f3Invoked && f3DisabledAgain;
        osnapStateRoundTrip = mouseDisabled && keyboardRestored
            && keyboardDisabled && mouseRestored
            && osnapNonObjectBitsPreserved && f3ActualKeyEvents;
        osnapMouseKeyboardSynchronized = osnapStateRoundTrip;
        osnapPriorSetPersisted = persistedObjectBits
            == (configuredBits & objectSnapMask);

        RS_SnapMode trackingOff = actionHandler->getSnaps();
        trackingOff.snapTracking = false;
        actionHandler->slotSetSnaps(trackingOff);
        {
            const QSignalBlocker blocker(trackingStatusButton);
            trackingStatusButton->setChecked(false);
        }
        trackingStatusButton->click();
        QApplication::processEvents();
        const bool mouseEnabledTracking =
            actionHandler->getSnaps().snapTracking
            && trackingStatusButton->isChecked();
        const bool f11Invoked = trackingF11Registered
            && sendFunctionKey(Qt::Key_F11);
        const bool keyboardDisabledTracking = f11Invoked
            && !actionHandler->getSnaps().snapTracking
            && !trackingStatusButton->isChecked();
        const bool f11Restored = trackingF11Registered
            && sendFunctionKey(Qt::Key_F11);
        const RS_SnapMode trackingRestored = actionHandler->getSnaps();
        const bool keyboardEnabledTracking = f11Restored
            && trackingRestored.snapTracking
            && trackingStatusButton->isChecked();
        trackingSerializationRoundTrip = RS_SnapMode::fromInt(
            RS_SnapMode::toInt(trackingRestored)).snapTracking;
        f11ActualKeyEvents = f11Invoked && f11Restored;
        trackingStateRoundTrip = mouseEnabledTracking
            && keyboardDisabledTracking && keyboardEnabledTracking
            && f11ActualKeyEvents;
        trackingMouseKeyboardSynchronized = trackingStateRoundTrip;

        actionHandler->slotSetSnaps(original);
        {
            const QSignalBlocker osnapBlocker(osnapStatusButton);
            osnapStatusButton->setChecked(
                (RS_SnapMode::toInt(original) & objectSnapMask) != 0);
        }
        {
            const QSignalBlocker trackingBlocker(trackingStatusButton);
            trackingStatusButton->setChecked(original.snapTracking);
        }
        osnapStatusButton->setProperty("kuubikSavedOsnap",
                                       originalSavedProperty);
        if (hadSavedOsnap) {
            settings.setValue(savedOsnapKey, originalSavedOsnap);
        } else {
            settings.remove(savedOsnapKey);
        }
    }

    const QString objectSnapVisibilityKey = QStringLiteral(
        "KuubikStatus/Visible/ObjectSnap");
    const QString trackingVisibilityKey = QStringLiteral(
        "KuubikStatus/Visible/SnapTracking");
    QVariantMap existingVisibilityChoices{
        {objectSnapVisibilityKey, false},
        {trackingVisibilityKey, false}
    };
    applyPhaseThreeVisibilityDefaults(
        [&existingVisibilityChoices](const QString& key) {
            return existingVisibilityChoices.contains(key);
        },
        [&existingVisibilityChoices](const QString& key, bool value) {
            existingVisibilityChoices.insert(key, value);
        });
    const bool phaseThreePreservesExplicitHidden =
        !existingVisibilityChoices.value(objectSnapVisibilityKey).toBool()
        && !existingVisibilityChoices.value(trackingVisibilityKey).toBool();
    QVariantMap missingVisibilityChoices;
    applyPhaseThreeVisibilityDefaults(
        [&missingVisibilityChoices](const QString& key) {
            return missingVisibilityChoices.contains(key);
        },
        [&missingVisibilityChoices](const QString& key, bool value) {
            missingVisibilityChoices.insert(key, value);
        });
    const bool phaseThreeDefaultsMissingVisible =
        missingVisibilityChoices.value(objectSnapVisibilityKey).toBool()
        && missingVisibilityChoices.value(trackingVisibilityKey).toBool();

    const auto pointEvidence = [](const RS_Vector& actual,
                                  const RS_Vector& expected,
                                  double tolerance) {
        QJsonObject evidence;
        evidence.insert("actualX", actual.valid ? actual.x : 1.0e100);
        evidence.insert("actualY", actual.valid ? actual.y : 1.0e100);
        evidence.insert("expectedX", expected.x);
        evidence.insert("expectedY", expected.y);
        const double error = actual.valid
            ? actual.distanceTo(expected) : 1.0e100;
        evidence.insert("error", error);
        evidence.insert("passed", actual.valid && error <= tolerance);
        return evidence;
    };
    QJsonObject snapFamilies;
    bool snapFamiliesPassed = false;
    QJsonObject trackingGeometry;
    bool trackingGeometryPassed = false;
    if (referenceView != nullptr
        && referenceView->width() > 240 && referenceView->height() > 180) {
        constexpr double geometryTolerance = 1.0e-7;
        const RS_Vector originalRelativeZero = referenceView->getRelativeZero();

        const auto eventPoint = [referenceView](const RS_Vector& point) {
            const RS_Vector gui = referenceView->toGui(point);
            return QPoint(qRound(gui.x), qRound(gui.y));
        };
        const auto eventGraphPoint = [referenceView, &eventPoint](
                const RS_Vector& point) {
            const QPoint pixel = eventPoint(point);
            return referenceView->toGraph(pixel.x(), pixel.y());
        };
        const auto dispatchSnap = [referenceView, &eventPoint](
                RS_Graphic& fixture, const RS_SnapMode& mode,
                const RS_Vector& query) {
            RS_Snapper verifier(fixture, *referenceView);
            verifier.init();
            verifier.setSnapMode(mode);
            QMouseEvent event(QEvent::MouseMove, QPointF(eventPoint(query)),
                              Qt::NoButton, Qt::NoButton, Qt::NoModifier);
            const RS_Vector result = verifier.snapPoint(&event);
            verifier.finish();
            return result;
        };
        const auto modeFor = [](unsigned bit) {
            return RS_SnapMode::fromInt(bit);
        };

        RS_Graphic lineFixture;
        lineFixture.addEntity(new RS_Line(
            &lineFixture, RS_Vector(0.0, 0.0), RS_Vector(10.0, 0.0)));
        lineFixture.addEntity(new RS_Line(
            &lineFixture, RS_Vector(5.0, -5.0), RS_Vector(5.0, 5.0)));
        const RS_Vector endpoint = dispatchSnap(
            lineFixture, modeFor(RS_SnapMode::SnapEndpoint),
            RS_Vector(0.1, 0.1));
        const RS_Vector midpoint = dispatchSnap(
            lineFixture, modeFor(RS_SnapMode::SnapMiddle),
            RS_Vector(4.9, 0.1));
        const RS_Vector distance = dispatchSnap(
            lineFixture, modeFor(RS_SnapMode::SnapDistance),
            RS_Vector(1.1, 0.1));
        const RS_Vector intersection = dispatchSnap(
            lineFixture, modeFor(RS_SnapMode::SnapIntersection),
            RS_Vector(5.1, 0.1));
        const RS_Vector nearest = dispatchSnap(
            lineFixture, modeFor(RS_SnapMode::SnapOnEntity),
            RS_Vector(2.0, 2.0));
        const RS_Vector extensionQuery = eventGraphPoint(
            RS_Vector(12.0, 0.2));
        const RS_Vector extension = dispatchSnap(
            lineFixture, modeFor(RS_SnapMode::SnapExtension),
            extensionQuery);
        referenceView->setRelativeZero(RS_Vector(3.0, 4.0));
        const RS_Vector perpendicular = dispatchSnap(
            lineFixture, modeFor(RS_SnapMode::SnapPerpendicular),
            RS_Vector(3.0, 0.2));
        referenceView->setRelativeZero(RS_Vector(0.0, 10.0));
        const RS_Vector parallelQuery = eventGraphPoint(RS_Vector(2.0, 1.0));
        const RS_Vector parallel = dispatchSnap(
            lineFixture, modeFor(RS_SnapMode::SnapParallel), parallelQuery);
        const RS_Vector parallelExpected = RS_Vector(0.0, 10.0)
            + RS_Vector::polar(
                RS_Vector(0.0, 10.0).distanceTo(parallelQuery), 0.0);

        RS_Graphic circleFixture;
        circleFixture.addEntity(new RS_Circle(
            &circleFixture, RS_CircleData(RS_Vector(20.0, 0.0), 5.0)));
        const RS_Vector center = dispatchSnap(
            circleFixture, modeFor(RS_SnapMode::SnapCenter),
            RS_Vector(20.1, 0.1));
        const RS_Vector quadrant = dispatchSnap(
            circleFixture, modeFor(RS_SnapMode::SnapQuadrant),
            RS_Vector(25.1, 0.1));
        referenceView->setRelativeZero(RS_Vector(30.0, 0.0));
        const RS_Vector tangent = dispatchSnap(
            circleFixture, modeFor(RS_SnapMode::SnapTangent),
            RS_Vector(22.5, 4.3));

        RS_Graphic nodeFixture;
        nodeFixture.addEntity(new RS_Point(
            &nodeFixture, RS_PointData(RS_Vector(30.0, 5.0))));
        const RS_Vector node = dispatchSnap(
            nodeFixture, modeFor(RS_SnapMode::SnapNode),
            RS_Vector(30.1, 5.1));

        RS_Graphic insertionFixture;
        insertionFixture.addEntity(new RS_Insert(
            &insertionFixture,
            RS_InsertData(QStringLiteral("contract-insert"),
                          RS_Vector(40.0, 5.0), RS_Vector(1.0, 1.0),
                          0.0, 1, 1, RS_Vector(0.0, 0.0), nullptr,
                          RS2::NoUpdate)));
        const RS_Vector insertion = dispatchSnap(
            insertionFixture, modeFor(RS_SnapMode::SnapInsertion),
            RS_Vector(40.1, 5.1));

        RS_Graphic geometricCenterFixture;
        auto* rectangle = new RS_Polyline(&geometricCenterFixture);
        rectangle->addVertex(RS_Vector(50.0, 0.0));
        rectangle->addVertex(RS_Vector(60.0, 0.0));
        rectangle->addVertex(RS_Vector(60.0, 10.0));
        rectangle->addVertex(RS_Vector(50.0, 10.0));
        rectangle->setClosed(true);
        rectangle->endPolyline();
        geometricCenterFixture.addEntity(rectangle);
        const RS_Vector geometricCenter = dispatchSnap(
            geometricCenterFixture,
            modeFor(RS_SnapMode::SnapGeometricCenter),
            RS_Vector(55.1, 5.1));

        RS_Graphic apparentIntersectionFixture;
        apparentIntersectionFixture.addEntity(new RS_Line(
            &apparentIntersectionFixture, RS_Vector(70.0, 0.0),
            RS_Vector(74.0, 0.0)));
        apparentIntersectionFixture.addEntity(new RS_Line(
            &apparentIntersectionFixture, RS_Vector(76.0, 2.0),
            RS_Vector(76.0, 6.0)));
        const RS_Vector apparentIntersection = dispatchSnap(
            apparentIntersectionFixture,
            modeFor(RS_SnapMode::SnapApparentIntersection),
            RS_Vector(76.1, 0.1));
        referenceView->setRelativeZero(originalRelativeZero);

        snapFamilies.insert("dispatchPath",
                            "RS_Snapper::snapPoint(QMouseEvent*)");
        snapFamilies.insert("allFamiliesViaSnapPoint", true);
        snapFamilies.insert("helperOnlyFamilies", QJsonArray());
        snapFamilies.insert("endpoint", pointEvidence(
            endpoint, RS_Vector(0.0, 0.0), geometryTolerance));
        snapFamilies.insert("midpoint", pointEvidence(
            midpoint, RS_Vector(5.0, 0.0), geometryTolerance));
        snapFamilies.insert("distance", pointEvidence(
            distance, RS_Vector(1.0, 0.0), geometryTolerance));
        snapFamilies.insert("center", pointEvidence(
            center, RS_Vector(20.0, 0.0), geometryTolerance));
        snapFamilies.insert("quadrant", pointEvidence(
            quadrant, RS_Vector(25.0, 0.0), geometryTolerance));
        snapFamilies.insert("intersection", pointEvidence(
            intersection, RS_Vector(5.0, 0.0), geometryTolerance));
        snapFamilies.insert("node", pointEvidence(
            node, RS_Vector(30.0, 5.0), geometryTolerance));
        snapFamilies.insert("insertion", pointEvidence(
            insertion, RS_Vector(40.0, 5.0), geometryTolerance));
        snapFamilies.insert("perpendicular", pointEvidence(
            perpendicular, RS_Vector(3.0, 0.0), geometryTolerance));
        snapFamilies.insert("tangent", pointEvidence(
            tangent, RS_Vector(22.5, 5.0 * std::sqrt(3.0) / 2.0),
            geometryTolerance));
        snapFamilies.insert("geometricCenter", pointEvidence(
            geometricCenter, RS_Vector(55.0, 5.0), geometryTolerance));
        snapFamilies.insert("apparentIntersection", pointEvidence(
            apparentIntersection, RS_Vector(76.0, 0.0),
            geometryTolerance));
        snapFamilies.insert("nearest", pointEvidence(
            nearest, RS_Vector(eventGraphPoint(RS_Vector(2.0, 2.0)).x, 0.0),
            geometryTolerance));
        snapFamilies.insert("extension", pointEvidence(
            extension, RS_Vector(extensionQuery.x, 0.0), geometryTolerance));
        snapFamilies.insert("parallel", pointEvidence(
            parallel, parallelExpected, geometryTolerance));
        snapFamiliesPassed = true;
        for (auto it = snapFamilies.constBegin();
             it != snapFamilies.constEnd(); ++it) {
            if (it.value().isObject()) {
                snapFamiliesPassed = snapFamiliesPassed
                    && it.value().toObject().value("passed").toBool();
            }
        }

        const QPoint acquisitionPixel(referenceView->width() / 2,
                                      referenceView->height() / 2);
        const RS_Vector acquisitionPoint = referenceView->toGraph(
            acquisitionPixel.x(), acquisitionPixel.y());
        RS_Graphic trackingFixture;
        trackingFixture.addEntity(new RS_Point(
            &trackingFixture, RS_PointData(acquisitionPoint)));
        const unsigned trackingEntityCount = trackingFixture.count();
        const int trackingUndoCyclesBefore =
            trackingFixture.countUndoCycles();
        RS_Snapper trackingVerifier(trackingFixture, *referenceView);
        trackingVerifier.init();
        RS_SnapMode trackingMode;
        trackingMode.snapNode = true;
        trackingMode.snapFree = false;
        trackingMode.snapTracking = true;
        trackingVerifier.setSnapMode(trackingMode);
        QMouseEvent acquisitionEvent(
            QEvent::MouseMove, QPointF(acquisitionPixel), Qt::NoButton,
            Qt::NoButton, Qt::NoModifier);
        const RS_Vector acquiredResult = trackingVerifier.snapPoint(
            &acquisitionEvent);
        const bool candidateAcquired = trackingVerifier.hasTrackingAcquisition()
            && acquiredResult.distanceTo(acquisitionPoint)
                   <= geometryTolerance;
        const bool acquisitionKeyEntityPresent =
            trackingVerifier.getKeyEntity() != nullptr;

        const QPoint orthogonalPixel = acquisitionPixel + QPoint(80, -4);
        const RS_Vector orthogonalRaw = referenceView->toGraph(
            orthogonalPixel.x(), orthogonalPixel.y());
        QMouseEvent orthogonalEvent(
            QEvent::MouseMove, QPointF(orthogonalPixel), Qt::NoButton,
            Qt::NoButton, Qt::NoModifier);
        const RS_Vector orthogonalResult = trackingVerifier.snapPoint(
            &orthogonalEvent);
        const RS_Vector orthogonalExpected(orthogonalRaw.x,
                                           acquisitionPoint.y);
        const double trackingTolerance = std::max(
            geometryTolerance, std::abs(referenceView->toGraphDX(0.25)));
        const QJsonObject orthogonalEvidence = pointEvidence(
            orthogonalResult, orthogonalExpected, trackingTolerance);
        const auto overlayGuideEvidence = [referenceView, &pointEvidence](
                const RS_Vector& acquired, const RS_Vector& projected) {
            QJsonObject evidence;
            evidence.insert("overlayContainer", "RS2::Snapper");
            evidence.insert("entityType", "RS_OverlayLine");
            const RS_Vector expectedStart = referenceView->toGui(acquired);
            const RS_Vector expectedEnd = referenceView->toGui(projected);
            RS_Vector actualStart(false);
            RS_Vector actualEnd(false);
            int overlayLineCount = 0;
            auto* overlay = referenceView->getOverlayContainer(RS2::Snapper);
            for (RS_Entity* entity : overlay->getEntityList()) {
                auto* line = dynamic_cast<RS_OverlayLine*>(entity);
                if (line == nullptr
                    || line->rtti() != RS2::EntityOverlayLine) {
                    continue;
                }
                ++overlayLineCount;
                const RS_Vector start = line->getStartpoint();
                const RS_Vector end = line->getEndpoint();
                const bool direct = start.distanceTo(expectedStart) <= 1.0e-7
                    && end.distanceTo(expectedEnd) <= 1.0e-7;
                const bool reverse = start.distanceTo(expectedEnd) <= 1.0e-7
                    && end.distanceTo(expectedStart) <= 1.0e-7;
                if (direct || reverse) {
                    actualStart = direct ? start : end;
                    actualEnd = direct ? end : start;
                    break;
                }
            }
            const QJsonObject startEvidence = pointEvidence(
                actualStart, expectedStart, 1.0e-7);
            const QJsonObject endEvidence = pointEvidence(
                actualEnd, expectedEnd, 1.0e-7);
            const bool passed = startEvidence.value("passed").toBool()
                && endEvidence.value("passed").toBool();
            evidence.insert("overlayLineCount", overlayLineCount);
            evidence.insert("lineFound", passed);
            evidence.insert("start", startEvidence);
            evidence.insert("end", endEvidence);
            evidence.insert("passed", passed);
            return evidence;
        };
        const QJsonObject orthogonalOverlay = overlayGuideEvidence(
            acquisitionPoint, orthogonalResult);
        const bool orthogonalGuide = trackingVerifier.hasTrackingGuide()
            && trackingVerifier.trackingAcquisition().distanceTo(
                   acquisitionPoint) <= trackingTolerance
            && trackingVerifier.trackingGuideEnd().distanceTo(
                   orthogonalResult) <= trackingTolerance
            && orthogonalOverlay.value("passed").toBool();
        const bool trackingKeyEntityIsNull =
            trackingVerifier.getKeyEntity() == nullptr;

        const RS_Vector competingEndpoint = referenceView->toGraph(
            acquisitionPixel.x() + 80, acquisitionPixel.y() - 20);
        trackingFixture.addEntity(new RS_Point(
            &trackingFixture, RS_PointData(competingEndpoint)));
        const unsigned entitiesBeforePriorityChecks = trackingFixture.count();
        QMouseEvent objectPriorityEvent(
            QEvent::MouseMove,
            QPointF(acquisitionPixel + QPoint(80, -20)), Qt::NoButton,
            Qt::NoButton, Qt::NoModifier);
        const RS_Vector objectPriorityResult = trackingVerifier.snapPoint(
            &objectPriorityEvent);
        const bool objectSnapPrecedence = objectPriorityResult.distanceTo(
            competingEndpoint) <= trackingTolerance
            && !trackingVerifier.hasTrackingGuide();

        trackingVerifier.setSnapMode(trackingMode);
        trackingVerifier.snapPoint(&acquisitionEvent);
        RS_SnapMode gridTrackingMode = trackingMode;
        gridTrackingMode.snapGrid = true;
        trackingVerifier.setSnapMode(gridTrackingMode);
        const RS_Vector provisionalGridRaw = referenceView->toGraph(
            acquisitionPixel.x() + 140, acquisitionPixel.y() - 4);
        const RS_Vector gridPoint = trackingVerifier.snapGrid(
            provisionalGridRaw);
        const RS_Vector gridGui = referenceView->toGui(gridPoint);
        QMouseEvent gridPriorityEvent(
            QEvent::MouseMove, QPointF(gridGui.x, gridGui.y), Qt::NoButton,
            Qt::NoButton, Qt::NoModifier);
        const RS_Vector gridRaw = referenceView->toGraph(
            static_cast<int>(gridGui.x), static_cast<int>(gridGui.y));
        const RS_Vector gridExpected = trackingVerifier.snapGrid(gridRaw);
        const RS_Vector gridPriorityResult = trackingVerifier.snapPoint(
            &gridPriorityEvent);
        const bool gridPrecedence = gridPriorityResult.distanceTo(gridExpected)
                <= trackingTolerance
            && !trackingVerifier.hasTrackingGuide();

        trackingVerifier.setSnapMode(trackingMode);
        trackingVerifier.snapPoint(&acquisitionEvent);
        RS_SnapMode orthoTrackingMode = trackingMode;
        orthoTrackingMode.restriction = RS2::RestrictOrthogonal;
        trackingVerifier.setSnapMode(orthoTrackingMode);
        const QPoint orthoPixel = acquisitionPixel + QPoint(120, -25);
        const RS_Vector orthoRaw = referenceView->toGraph(
            orthoPixel.x(), orthoPixel.y());
        const RS_Vector orthoExpected = trackingVerifier.restrictOrthogonal(
            orthoRaw);
        QMouseEvent orthoPriorityEvent(
            QEvent::MouseMove, QPointF(orthoPixel), Qt::NoButton,
            Qt::NoButton, Qt::NoModifier);
        const RS_Vector orthoPriorityResult = trackingVerifier.snapPoint(
            &orthoPriorityEvent);
        const bool orthoPrecedence = orthoPriorityResult.distanceTo(
            orthoExpected) <= trackingTolerance
            && !trackingVerifier.hasTrackingGuide();

        QString originalTrackingIncrement;
        {
            auto settingsGuard = RS_SETTINGS->beginGroupGuard("/Snap");
            originalTrackingIncrement = RS_SETTINGS->readEntry(
                "/AngleIncrement", "15");
            RS_SETTINGS->writeEntry("/AngleIncrement", QStringLiteral("45"));
        }
        trackingVerifier.setSnapMode(trackingMode);
        trackingVerifier.snapPoint(&acquisitionEvent);
        RS_SnapMode polarTrackingMode = trackingMode;
        polarTrackingMode.snapAngle = true;
        trackingVerifier.setSnapMode(polarTrackingMode);
        const QPoint polarPixel = acquisitionPixel + QPoint(80, -74);
        const RS_Vector polarRaw = referenceView->toGraph(
            polarPixel.x(), polarPixel.y());
        const double polarRadians = M_PI / 4.0;
        const RS_Vector polarDirection(std::cos(polarRadians),
                                       std::sin(polarRadians));
        const RS_Vector polarDelta = polarRaw - acquisitionPoint;
        const double polarAlong = polarDelta.x * polarDirection.x
            + polarDelta.y * polarDirection.y;
        const RS_Vector polarExpected = acquisitionPoint
            + polarDirection * polarAlong;
        QMouseEvent polarEvent(
            QEvent::MouseMove, QPointF(polarPixel), Qt::NoButton,
            Qt::NoButton, Qt::NoModifier);
        const RS_Vector polarResult = trackingVerifier.snapPoint(&polarEvent);
        const QJsonObject polarEvidence = pointEvidence(
            polarResult, polarExpected, trackingTolerance);
        const QJsonObject polarOverlay = overlayGuideEvidence(
            acquisitionPoint, polarResult);
        const bool polarGuide = trackingVerifier.hasTrackingGuide()
            && polarOverlay.value("passed").toBool();
        const bool polarKeyEntityIsNull =
            trackingVerifier.getKeyEntity() == nullptr;
        {
            auto settingsGuard = RS_SETTINGS->beginGroupGuard("/Snap");
            RS_SETTINGS->writeEntry("/AngleIncrement",
                                    originalTrackingIncrement);
        }

        const bool overlayDrawnBeforeSuspend =
            referenceView->getOverlayContainer(RS2::Snapper)->count() > 0;
        trackingVerifier.suspend();
        const bool suspendClearsDrawnOverlayImmediately =
            overlayDrawnBeforeSuspend
            && !trackingVerifier.hasTrackingAcquisition()
            && !trackingVerifier.hasTrackingGuide()
            && referenceView->getOverlayContainer(RS2::Snapper)->count() == 0;

        trackingVerifier.setSnapMode(polarTrackingMode);
        trackingVerifier.snapPoint(&acquisitionEvent);
        trackingVerifier.snapPoint(&polarEvent);
        const bool guideDrawnBeforeDisable = trackingVerifier.hasTrackingGuide()
            && referenceView->getOverlayContainer(RS2::Snapper)->count() > 0;
        RS_SnapMode trackingDisabled = polarTrackingMode;
        trackingDisabled.snapTracking = false;
        trackingVerifier.setSnapMode(trackingDisabled);
        const bool disablingClearsTracking = guideDrawnBeforeDisable
            && !trackingVerifier.hasTrackingAcquisition()
            && !trackingVerifier.hasTrackingGuide()
            && referenceView->getOverlayContainer(RS2::Snapper)->count() == 0;
        const bool documentUnchanged = trackingFixture.count()
            == entitiesBeforePriorityChecks
            && trackingEntityCount + 1 == entitiesBeforePriorityChecks;
        const int trackingUndoCyclesAfter =
            trackingFixture.countUndoCycles();
        const bool undoStateUnchanged = trackingUndoCyclesAfter
            == trackingUndoCyclesBefore;
        trackingVerifier.finish();

        trackingGeometry.insert("snapFreeDisabled", !trackingMode.snapFree);
        trackingGeometry.insert("candidateAcquired", candidateAcquired);
        trackingGeometry.insert("acquisitionKeyEntityPresent",
                                acquisitionKeyEntityPresent);
        trackingGeometry.insert("orthogonalProjection", orthogonalEvidence);
        trackingGeometry.insert("orthogonalProjectionWithSnapFreeDisabled",
                                orthogonalEvidence.value("passed").toBool());
        trackingGeometry.insert("orthogonalGuideVisible", orthogonalGuide);
        trackingGeometry.insert("orthogonalOverlayLine", orthogonalOverlay);
        trackingGeometry.insert("polarProjection", polarEvidence);
        trackingGeometry.insert("polarProjectionWithSnapFreeDisabled",
                                polarEvidence.value("passed").toBool());
        trackingGeometry.insert("polarGuideVisible", polarGuide);
        trackingGeometry.insert("polarOverlayLine", polarOverlay);
        trackingGeometry.insert("keyEntityNullForTracking",
            acquisitionKeyEntityPresent && trackingKeyEntityIsNull
                && polarKeyEntityIsNull);
        trackingGeometry.insert("objectSnapPrecedence", objectSnapPrecedence);
        trackingGeometry.insert("gridSnapPrecedence", gridPrecedence);
        trackingGeometry.insert("orthoRestrictionPrecedence", orthoPrecedence);
        trackingGeometry.insert("suspendClearsDrawnOverlayImmediately",
                                suspendClearsDrawnOverlayImmediately);
        trackingGeometry.insert("disabledClearsAcquisition",
                                disablingClearsTracking);
        trackingGeometry.insert("documentEntityCountBefore",
                                static_cast<int>(entitiesBeforePriorityChecks));
        trackingGeometry.insert("documentEntityCountAfter",
                                static_cast<int>(trackingFixture.count()));
        trackingGeometry.insert("documentEntitiesUnchanged", documentUnchanged);
        trackingGeometry.insert("undoCycleCountBefore",
                                trackingUndoCyclesBefore);
        trackingGeometry.insert("undoCycleCountAfter",
                                trackingUndoCyclesAfter);
        trackingGeometry.insert("undoStateUnchanged", undoStateUnchanged);
        trackingGeometry.insert("overlayOnly", documentUnchanged
            && undoStateUnchanged && orthogonalGuide && polarGuide);
        trackingGeometryPassed = !trackingMode.snapFree && candidateAcquired
            && acquisitionKeyEntityPresent && trackingKeyEntityIsNull
            && orthogonalEvidence.value("passed").toBool()
            && orthogonalGuide && polarEvidence.value("passed").toBool()
            && polarGuide && polarKeyEntityIsNull
            && objectSnapPrecedence && gridPrecedence
            && orthoPrecedence && suspendClearsDrawnOverlayImmediately
            && disablingClearsTracking
            && documentUnchanged && undoStateUnchanged;
        trackingGeometry.insert("passed", trackingGeometryPassed);
    }

    snapFamilies.insert("passed", snapFamiliesPassed);
    QJsonObject referenceTwelveToThirteen;
    referenceTwelveToThirteen.insert("referenceStartPage", 12);
    referenceTwelveToThirteen.insert("referenceEndPage", 13);
    referenceTwelveToThirteen.insert("referencePageCount", 2);
    referenceTwelveToThirteen.insert("osnapReferencePage",
        osnapStatusButton == nullptr ? 0
            : osnapStatusButton->property("kuubikReferencePage").toInt());
    referenceTwelveToThirteen.insert("trackingReferencePage",
        trackingStatusButton == nullptr ? 0
            : trackingStatusButton->property("kuubikReferencePage").toInt());
    referenceTwelveToThirteen.insert("osnapControlVisible",
        osnapStatusButton != nullptr && !osnapStatusButton->isHidden());
    referenceTwelveToThirteen.insert("trackingControlVisible",
        trackingStatusButton != nullptr && !trackingStatusButton->isHidden());
    referenceTwelveToThirteen.insert("osnapStateRoundTrip",
                                     osnapStateRoundTrip);
    referenceTwelveToThirteen.insert("osnapMouseKeyboardSynchronized",
                                     osnapMouseKeyboardSynchronized);
    referenceTwelveToThirteen.insert("osnapPriorSetPersisted",
                                     osnapPriorSetPersisted);
    referenceTwelveToThirteen.insert("osnapNonObjectBitsPreserved",
                                     osnapNonObjectBitsPreserved);
    referenceTwelveToThirteen.insert("f3ActualKeyEvents",
                                     f3ActualKeyEvents);
    referenceTwelveToThirteen.insert("trackingStateRoundTrip",
                                     trackingStateRoundTrip);
    referenceTwelveToThirteen.insert("trackingMouseKeyboardSynchronized",
                                     trackingMouseKeyboardSynchronized);
    referenceTwelveToThirteen.insert("trackingSerializationRoundTrip",
                                     trackingSerializationRoundTrip);
    referenceTwelveToThirteen.insert("f11ActualKeyEvents",
                                     f11ActualKeyEvents);
    referenceTwelveToThirteen.insert("phaseThreePreservesExplicitHidden",
                                     phaseThreePreservesExplicitHidden);
    referenceTwelveToThirteen.insert("phaseThreeDefaultsMissingVisible",
                                     phaseThreeDefaultsMissingVisible);
    referenceTwelveToThirteen.insert("snapFamilies", snapFamilies);
    referenceTwelveToThirteen.insert("trackingGeometry", trackingGeometry);
    referenceTwelveToThirteen.insert("passed", snapFamiliesPassed
        && trackingGeometryPassed && osnapStateRoundTrip
        && trackingStateRoundTrip && osnapPriorSetPersisted
        && osnapNonObjectBitsPreserved && f3ActualKeyEvents
        && f11ActualKeyEvents && phaseThreePreservesExplicitHidden
        && phaseThreeDefaultsMissingVisible);
    statusBarObject.insert("referencePdfPagesTwelveToThirteen",
                           referenceTwelveToThirteen);

    QJsonObject referencePageThirtyThree;
    referencePageThirtyThree.insert("f3Registered", osnapF3Registered);
    referencePageThirtyThree.insert("f11Registered", trackingF11Registered);
    referencePageThirtyThree.insert("fullscreenF11Removed",
                                    fullscreenF11Removed);
    referencePageThirtyThree.insert("f3UsesOsnapMainToggle",
                                    osnapMouseKeyboardSynchronized);
    referencePageThirtyThree.insert("f11UsesTrackingToggle",
                                    trackingMouseKeyboardSynchronized);
    referencePageThirtyThree.insert("f3ActualKeyEvents", f3ActualKeyEvents);
    referencePageThirtyThree.insert("f11ActualKeyEvents", f11ActualKeyEvents);
    referencePageThirtyThree.insert("f3PreservesNonObjectSnapBits",
                                    osnapNonObjectBitsPreserved);
    referencePageThirtyThree.insert("passed", osnapF3Registered
        && trackingF11Registered && fullscreenF11Removed
        && osnapMouseKeyboardSynchronized
        && trackingMouseKeyboardSynchronized && f3ActualKeyEvents
        && f11ActualKeyEvents && osnapNonObjectBitsPreserved);
    statusBarObject.insert("referencePdfPageThirtyThreeShortcuts",
                           referencePageThirtyThree);
    contract.insert("statusBar", statusBarObject);

    QJsonObject dpiObject;
    dpiObject.insert("logicalDpiX", logicalDpiX());
    dpiObject.insert("logicalDpiY", logicalDpiY());
    dpiObject.insert("devicePixelRatio", devicePixelRatioF());
    dpiObject.insert("windowLogicalWidth", width());
    dpiObject.insert("windowLogicalHeight", height());
    bool scaleOk = false;
    const double requestedScaleFactor = qEnvironmentVariable("QT_SCALE_FACTOR").toDouble(&scaleOk);
    dpiObject.insert("requestedScaleFactor", scaleOk ? requestedScaleFactor : 1.0);
    const QString screenshotPath = qEnvironmentVariable("KUUBIK_UI_SCREENSHOT_PATH");
    bool screenshotSaved = screenshotPath.isEmpty();
    int screenshotPixelWidth = 0;
    int screenshotPixelHeight = 0;
    double screenshotDevicePixelRatio = 0.0;
    if (!screenshotPath.isEmpty()) {
        const QPixmap screenshot = grab();
        screenshotPixelWidth = screenshot.width();
        screenshotPixelHeight = screenshot.height();
        screenshotDevicePixelRatio = screenshot.devicePixelRatioF();
        screenshotSaved = screenshot.save(screenshotPath, "PNG");
    }
    dpiObject.insert("screenshotSaved", screenshotSaved);
    dpiObject.insert("screenshotPixelWidth", screenshotPixelWidth);
    dpiObject.insert("screenshotPixelHeight", screenshotPixelHeight);
    dpiObject.insert("screenshotDevicePixelRatio", screenshotDevicePixelRatio);
    contract.insert("dpi", dpiObject);

    QJsonObject colors;
    const auto colorTokens = KuubikTheme::colors();
    for (auto it = colorTokens.cbegin(); it != colorTokens.cend(); ++it) {
        colors.insert(it.key(), it.value());
    }
    contract.insert("colors", colors);

    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    file.write(QJsonDocument(contract).toJson(QJsonDocument::Indented));
    return file.commit() && screenshotSaved;
}

bool QC_ApplicationWindow::runKuubikToolOptionsSmoke(
    const QString& outputDirectory)
{
    QDir output(outputDirectory);
    if (!output.exists() && !QDir().mkpath(output.absolutePath())) {
        return false;
    }

    setWindowState(windowState() & ~Qt::WindowMaximized);
    resize(1280, 600);
    applyKuubikWorkspace(false);
    show();
    QApplication::processEvents();

    QWidget* toolbarHost = findChild<QWidget*>(
        QStringLiteral("kuubikOptionToolbarHost"));
    const auto containedBy = [](QWidget* child, QWidget* ancestor) {
        if (child == nullptr || ancestor == nullptr) return false;
        const QRect mappedRect(child->mapTo(ancestor, QPoint(0, 0)), child->size());
        return ancestor->rect().contains(mappedRect);
    };
    const auto containedThroughAncestors = [](QWidget* child, QWidget* root) {
        if (child == nullptr || root == nullptr) return false;
        QWidget* current = child;
        while (current != root) {
            QWidget* parent = current->parentWidget();
            if (parent == nullptr) return false;
            const QRect mappedRect(current->mapTo(parent, QPoint(0, 0)),
                                   current->size());
            if (!parent->rect().contains(mappedRect)) return false;
            current = parent;
        }
        return true;
    };
    const auto geometryObject = [](QWidget* widget) {
        QJsonObject geometry;
        geometry.insert(QStringLiteral("x"), widget == nullptr ? 0 : widget->x());
        geometry.insert(QStringLiteral("y"), widget == nullptr ? 0 : widget->y());
        geometry.insert(QStringLiteral("width"), widget == nullptr ? 0 : widget->width());
        geometry.insert(QStringLiteral("height"), widget == nullptr ? 0 : widget->height());
        return geometry;
    };

    QJsonObject report;
    report.insert(QStringLiteral("schemaVersion"), 1);
    report.insert(QStringLiteral("platform"),
                  qEnvironmentVariable("QT_QPA_PLATFORM"));
    report.insert(QStringLiteral("windowWidth"), width());
    report.insert(QStringLiteral("windowHeight"), height());
    report.insert(QStringLiteral("devicePixelRatio"), devicePixelRatioF());

    QJsonObject toolbarObject;
    toolbarObject.insert(QStringLiteral("present"), optionWidget != nullptr);
    toolbarObject.insert(QStringLiteral("objectName"),
                         optionWidget == nullptr ? QString() : optionWidget->objectName());
    toolbarObject.insert(QStringLiteral("visible"),
                         optionWidget != nullptr && optionWidget->isVisible());
    toolbarObject.insert(QStringLiteral("hostPresent"), toolbarHost != nullptr);
    toolbarObject.insert(QStringLiteral("hostObjectName"),
                         toolbarHost == nullptr ? QString() : toolbarHost->objectName());
    toolbarObject.insert(QStringLiteral("hostVisible"),
                         toolbarHost != nullptr && toolbarHost->isVisible());
    toolbarObject.insert(QStringLiteral("nativeToolbarInRibbon"),
                         optionWidget != nullptr && kuubikRibbon != nullptr
                             && kuubikRibbon->isAncestorOf(optionWidget));
    toolbarObject.insert(QStringLiteral("directChildOfHost"),
                         optionWidget != nullptr
                             && optionWidget->parentWidget() == toolbarHost);
    toolbarObject.insert(QStringLiteral("containedByHost"),
                         containedBy(optionWidget, toolbarHost));
    toolbarObject.insert(QStringLiteral("containedThroughWindowAncestors"),
                         containedThroughAncestors(optionWidget, this));
    toolbarObject.insert(QStringLiteral("idlePositiveSize"),
                         optionWidget != nullptr && optionWidget->width() > 0
                             && optionWidget->height() > 0 && toolbarHost != nullptr
                             && toolbarHost->width() > 0 && toolbarHost->height() > 0);
    toolbarObject.insert(QStringLiteral("geometry"), geometryObject(optionWidget));
    toolbarObject.insert(QStringLiteral("hostGeometry"), geometryObject(toolbarHost));
    report.insert(QStringLiteral("optionsToolbar"), toolbarObject);

    bool allPassed = optionWidget != nullptr && toolbarHost != nullptr
                     && optionWidget->objectName() == QStringLiteral("options_toolbar")
                     && toolbarHost->objectName()
                            == QStringLiteral("kuubikOptionToolbarHost")
                     && optionWidget->isVisible() && toolbarHost->isVisible()
                     && kuubikRibbon != nullptr
                     && kuubikRibbon->isAncestorOf(optionWidget)
                     && optionWidget->parentWidget() == toolbarHost
                     && containedBy(optionWidget, toolbarHost)
                     && containedThroughAncestors(optionWidget, this);

    const auto captureState = [&](const QString& actionKey,
                                  RS2::ActionType expectedActionType,
                                  const QStringList& expectedWidgetNames,
                                  const QString& screenshotName) {
        slotKillAllActions();
        QApplication::processEvents();
        QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
        QApplication::processEvents();

        QAction* action = a_map.value(actionKey, nullptr);
        QToolButton* ribbonButton = kuubikRibbon == nullptr
                                        ? nullptr
                                        : kuubikRibbon->buttonForAction(actionKey);
        if (action != nullptr && action->isEnabled()) {
            action->trigger();
            QApplication::processEvents();
            // QAction setup can replace an option widget more than once. A
            // real user click returns to the event loop, where deleteLater()
            // removes the replaced QWidgetAction content. Flush that queue
            // before measuring so the smoke observes the settled UI state.
            QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
            QApplication::processEvents();
        }

        QC_MDIWindow* mdi = getMDIWindow();
        RS_ActionInterface* activeAction = mdi == nullptr
                                               || mdi->getEventHandler() == nullptr
                                           ? nullptr
                                           : mdi->getEventHandler()->getCurrentAction();
        const bool actionActive = activeAction != nullptr
                                  && activeAction->rtti() == expectedActionType;
        const bool ribbonIdentity = action != nullptr && ribbonButton != nullptr
                                    && ribbonButton->defaultAction() == action
                                    && ribbonButton->property("kuubikActionKey").toString()
                                           == actionKey;
        const bool toolbarPositiveSize = optionWidget != nullptr
                                         && optionWidget->width() > 0
                                         && optionWidget->height() > 0;
        const bool hostPositiveSize = toolbarHost != nullptr
                                      && toolbarHost->width() > 0
                                      && toolbarHost->height() > 0;
        const bool toolbarInsideHost = containedBy(optionWidget, toolbarHost);
        const bool toolbarInsideAllAncestors = containedThroughAncestors(
            optionWidget, this);

        int visibleLineOptions = 0;
        int visibleDimensionOptions = 0;
        int visibleDimLinearOptions = 0;
        if (optionWidget != nullptr) {
            for (QG_LineOptions* widget
                 : optionWidget->findChildren<QG_LineOptions*>()) {
                if (widget->isVisible()) ++visibleLineOptions;
            }
            for (QG_DimOptions* widget
                 : optionWidget->findChildren<QG_DimOptions*>()) {
                if (widget->isVisible()) ++visibleDimensionOptions;
            }
            for (QG_DimLinearOptions* widget
                 : optionWidget->findChildren<QG_DimLinearOptions*>()) {
                if (widget->isVisible()) ++visibleDimLinearOptions;
            }
        }
        const bool settledWidgetCounts = actionKey == QStringLiteral("DrawLine")
                                             ? visibleLineOptions == 1
                                                   && visibleDimensionOptions == 0
                                                   && visibleDimLinearOptions == 0
                                             : visibleLineOptions == 0
                                                   && visibleDimensionOptions == 1
                                                   && visibleDimLinearOptions == 1;

        QJsonArray widgetObjects;
        bool widgetsPassed = true;
        for (const QString& objectName : expectedWidgetNames) {
            QWidget* widget = optionWidget == nullptr
                                  ? nullptr
                                  : optionWidget->findChild<QWidget*>(objectName);
            bool nativeType = false;
            if (objectName == QStringLiteral("Ui_LineOptions")) {
                nativeType = qobject_cast<QG_LineOptions*>(widget) != nullptr;
            } else if (objectName == QStringLiteral("Ui_DimOptions")) {
                nativeType = qobject_cast<QG_DimOptions*>(widget) != nullptr;
            } else if (objectName == QStringLiteral("Ui_DimLinearOptions")) {
                nativeType = qobject_cast<QG_DimLinearOptions*>(widget) != nullptr;
            }
            const bool visible = widget != nullptr && widget->isVisible();
            const bool insideToolbar = containedBy(widget, optionWidget);
            const bool insideHost = containedBy(widget, toolbarHost);
            const bool insideWindow = containedBy(widget, this);
            const bool insideAllAncestors = containedThroughAncestors(widget, this);
            const bool positiveSize = widget != nullptr && widget->width() > 0
                                      && widget->height() > 0;
            const bool widgetPassed = nativeType && visible && insideToolbar
                                      && insideHost && insideWindow
                                      && insideAllAncestors && positiveSize;
            widgetsPassed = widgetsPassed && widgetPassed;

            QJsonObject widgetObject;
            widgetObject.insert(QStringLiteral("objectName"), objectName);
            widgetObject.insert(QStringLiteral("present"), widget != nullptr);
            widgetObject.insert(QStringLiteral("nativeType"), nativeType);
            widgetObject.insert(QStringLiteral("visible"), visible);
            widgetObject.insert(QStringLiteral("containedByToolbar"), insideToolbar);
            widgetObject.insert(QStringLiteral("containedByHost"), insideHost);
            widgetObject.insert(QStringLiteral("containedByWindow"), insideWindow);
            widgetObject.insert(QStringLiteral("containedThroughWindowAncestors"),
                                insideAllAncestors);
            widgetObject.insert(QStringLiteral("positiveSize"), positiveSize);
            widgetObject.insert(QStringLiteral("geometry"), geometryObject(widget));
            widgetObject.insert(QStringLiteral("passed"), widgetPassed);
            widgetObjects.append(widgetObject);
        }

        const QString screenshotPath = output.filePath(screenshotName);
        const QPixmap screenshot = grab();
        const bool screenshotSaved = screenshot.save(screenshotPath, "PNG");
        const bool screenshotSizeValid = screenshot.width() == 1280
                                         && screenshot.height() == 600;
        const bool statePassed = action != nullptr && action->isEnabled()
                                 && ribbonIdentity && actionActive
                                 && toolbarPositiveSize && hostPositiveSize
                                 && toolbarInsideHost && toolbarInsideAllAncestors
                                 && settledWidgetCounts && widgetsPassed && screenshotSaved
                                 && screenshotSizeValid;

        QJsonObject state;
        state.insert(QStringLiteral("actionKey"), actionKey);
        state.insert(QStringLiteral("actionPresent"), action != nullptr);
        state.insert(QStringLiteral("actionEnabled"),
                     action != nullptr && action->isEnabled());
        state.insert(QStringLiteral("ribbonIdentity"), ribbonIdentity);
        state.insert(QStringLiteral("expectedActionType"),
                     static_cast<int>(expectedActionType));
        state.insert(QStringLiteral("activeActionType"),
                     activeAction == nullptr ? static_cast<int>(RS2::ActionNone)
                                             : static_cast<int>(activeAction->rtti()));
        state.insert(QStringLiteral("nativeActionActive"), actionActive);
        state.insert(QStringLiteral("optionsToolbarGeometry"),
                     geometryObject(optionWidget));
        state.insert(QStringLiteral("optionsHostGeometry"),
                     geometryObject(toolbarHost));
        state.insert(QStringLiteral("optionsToolbarPositiveSize"),
                     toolbarPositiveSize);
        state.insert(QStringLiteral("optionsHostPositiveSize"), hostPositiveSize);
        state.insert(QStringLiteral("optionsToolbarContainedByHost"),
                     toolbarInsideHost);
        state.insert(QStringLiteral("optionsToolbarContainedThroughWindowAncestors"),
                     toolbarInsideAllAncestors);
        QJsonObject visibleWidgetCounts;
        visibleWidgetCounts.insert(QStringLiteral("line"), visibleLineOptions);
        visibleWidgetCounts.insert(QStringLiteral("dimension"),
                                   visibleDimensionOptions);
        visibleWidgetCounts.insert(QStringLiteral("dimLinear"),
                                   visibleDimLinearOptions);
        state.insert(QStringLiteral("visibleNativeWidgetCounts"),
                     visibleWidgetCounts);
        state.insert(QStringLiteral("settledWidgetCounts"), settledWidgetCounts);
        state.insert(QStringLiteral("widgets"), widgetObjects);
        state.insert(QStringLiteral("screenshot"), screenshotName);
        state.insert(QStringLiteral("screenshotSaved"), screenshotSaved);
        state.insert(QStringLiteral("screenshotPixelWidth"), screenshot.width());
        state.insert(QStringLiteral("screenshotPixelHeight"), screenshot.height());
        state.insert(QStringLiteral("screenshotDevicePixelRatio"),
                     screenshot.devicePixelRatioF());
        state.insert(QStringLiteral("passed"), statePassed);
        return state;
    };

    QJsonArray states;
    const QJsonObject lineState = captureState(
        QStringLiteral("DrawLine"), RS2::ActionDrawLine,
        {QStringLiteral("Ui_LineOptions")},
        QStringLiteral("tool-options-line-1280.png"));
    states.append(lineState);
    allPassed = allPassed && lineState.value(QStringLiteral("passed")).toBool();

    const QJsonObject dimLinearState = captureState(
        QStringLiteral("DimLinear"), RS2::ActionDimLinear,
        {QStringLiteral("Ui_DimOptions"), QStringLiteral("Ui_DimLinearOptions")},
        QStringLiteral("tool-options-dimlinear-1280.png"));
    states.append(dimLinearState);
    allPassed = allPassed && dimLinearState.value(QStringLiteral("passed")).toBool();
    report.insert(QStringLiteral("states"), states);

    slotKillAllActions();
    QApplication::processEvents();
    report.insert(QStringLiteral("status"),
                  allPassed ? QStringLiteral("PASS") : QStringLiteral("FAIL"));

    QSaveFile file(output.filePath(QStringLiteral("tool-options-1280.json")));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    file.write(QJsonDocument(report).toJson(QJsonDocument::Indented));
    return file.commit() && allPassed;
}

bool QC_ApplicationWindow::runKuubikGuiSmoke(const QString& outputDirectory)
{
    QDir output(outputDirectory);
    if (!output.exists() && !QDir().mkpath(output.absolutePath())) {
        return false;
    }

    // Keep the evidence viewport independent from the runner's physical
    // display. QWidget::grab() records the actual rendered widgets.
    setWindowState(windowState() & ~Qt::WindowMaximized);
    resize(1920, 1080);
    show();
    QApplication::processEvents();

    const QString inputDxfPath = qEnvironmentVariable("KUUBIK_GUI_SMOKE_INPUT_DXF");
    if (!inputDxfPath.isEmpty()) {
        slotFileOpen(inputDxfPath, RS2::FormatDXFRW);
        QApplication::processEvents();
    }

    auto* mdi = getMDIWindow();
    auto* view = mdi == nullptr ? nullptr : mdi->getGraphicView();
    auto* graphic = mdi == nullptr ? nullptr : mdi->getGraphic();
    auto* button = kuubikRibbon == nullptr
                       ? nullptr
                       : kuubikRibbon->buttonForAction(QStringLiteral("DrawLine"));
    auto* polylineButton = kuubikRibbon == nullptr
                               ? nullptr
                               : kuubikRibbon->buttonForAction(
                                     QStringLiteral("DrawPolyline"));
    auto* modifyDuplicateButton = kuubikRibbon == nullptr
                                       ? nullptr
                                       : kuubikRibbon->buttonForAction(
                                             QStringLiteral("ModifyDuplicate"));
    auto* modifyMoveButton = kuubikRibbon == nullptr
                                 ? nullptr
                                 : kuubikRibbon->buttonForAction(
                                       QStringLiteral("ModifyMove"));
    auto* undoQuickButton = kuubikRibbon == nullptr
                                ? nullptr
                                : kuubikRibbon->buttonForAction(
                                      QStringLiteral("EditUndo"));
    auto* redoQuickButton = kuubikRibbon == nullptr
                                ? nullptr
                                : kuubikRibbon->buttonForAction(
                                      QStringLiteral("EditRedo"));

    auto lineCount = [](RS_Graphic* currentGraphic) {
        int count = 0;
        if (currentGraphic != nullptr) {
            for (auto* entity : currentGraphic->getEntityList()) {
                if (entity != nullptr && entity->rtti() == RS2::EntityLine) {
                    ++count;
                }
            }
        }
        return count;
    };

    auto activeLineCount = [](RS_Graphic* currentGraphic) {
        int count = 0;
        if (currentGraphic != nullptr) {
            for (auto* entity : currentGraphic->getEntityList()) {
                if (entity != nullptr && entity->rtti() == RS2::EntityLine
                    && !entity->isUndone()) {
                    ++count;
                }
            }
        }
        return count;
    };

    auto activePolylineCount = [](RS_Graphic* currentGraphic) {
        int count = 0;
        if (currentGraphic != nullptr) {
            for (auto* entity : currentGraphic->getEntityList()) {
                if (entity != nullptr && entity->rtti() == RS2::EntityPolyline
                    && !entity->isUndone()) {
                    ++count;
                }
            }
        }
        return count;
    };

    auto sendMouseClick = [](QWidget* target, const QPoint& point,
                             Qt::MouseButton button) {
        QMouseEvent press(QEvent::MouseButtonPress, QPointF(point),
                          button, button, Qt::NoModifier);
        QApplication::sendEvent(target, &press);
        QMouseEvent release(QEvent::MouseButtonRelease, QPointF(point),
                            button, Qt::NoButton, Qt::NoModifier);
        QApplication::sendEvent(target, &release);
        QApplication::processEvents();
    };
    auto sendClick = [&sendMouseClick](QWidget* target, const QPoint& point) {
        sendMouseClick(target, point, Qt::LeftButton);
    };

    // A responsive ribbon panel may expose an action through its visible More
    // menu while the original item button is hidden. Exercise only a surface a
    // user can actually click: either that direct button or the visible
    // overflow button followed by the exact native QAction's menu geometry.
    auto invokeRibbonActionWithMouse = [&sendClick, this](
                                               QToolButton* sourceButton,
                                               QAction* expectedAction) {
        const bool sourceButtonVisible = sourceButton != nullptr
                                         && sourceButton->isVisibleTo(this);
        const bool sourceButtonEnabled = sourceButton != nullptr
                                         && sourceButton->isEnabled();
        const bool sourceButtonIdentity = sourceButton != nullptr
                                          && sourceButton->defaultAction()
                                                 == expectedAction;
        QFrame* panel = sourceButton == nullptr
                            ? nullptr
                            : qobject_cast<QFrame*>(sourceButton->parentWidget());
        const bool panelCollapsed = panel != nullptr
                                    && panel->property("kuubikCollapsed").toBool();
        QToolButton* overflowButton = panel == nullptr
                                          ? nullptr
                                          : panel->findChild<QToolButton*>(
                                                QStringLiteral(
                                                  "kuubikRibbonPanelOverflow"),
                                                Qt::FindDirectChildrenOnly);
        const bool overflowButtonVisible = overflowButton != nullptr
                                           && overflowButton->isVisibleTo(this);
        const bool overflowButtonEnabled = overflowButton != nullptr
                                           && overflowButton->isEnabled();
        QMenu* overflowMenu = overflowButton == nullptr
                                  ? nullptr
                                  : overflowButton->menu();
        const bool overflowActionIdentity = overflowMenu != nullptr
                                            && expectedAction != nullptr
                                            && overflowMenu->actions().contains(
                                                 expectedAction);

        bool actionTriggeredByMouse = false;
        bool overflowMenuInteractionRan = false;
        bool overflowMenuVisibleAfterOpen = false;
        bool overflowActionGeometryValid = false;
        bool overflowActionAtPoint = false;
        bool overflowMenuClosedAfterSelection = false;
        QString invocationSurface = QStringLiteral("unavailable");
        const QMetaObject::Connection triggerConnection = connect(
            expectedAction, &QAction::triggered, this,
            [&actionTriggeredByMouse](bool) {
                actionTriggeredByMouse = true;
            });

        if (sourceButtonVisible && sourceButtonEnabled && sourceButtonIdentity) {
            invocationSurface = QStringLiteral("directButton");
            sendClick(sourceButton, sourceButton->rect().center());
        } else if (!sourceButtonVisible && panelCollapsed
                   && overflowButtonVisible && overflowButtonEnabled
                   && overflowActionIdentity) {
            invocationSurface = QStringLiteral("collapsedPanelOverflow");
            // InstantPopup may run a nested menu event loop. Queue the menu-row
            // click first so the same helper works whether Qt opens the menu
            // synchronously or returns from the tool-button press immediately.
            QTimer menuClickTimer;
            menuClickTimer.setSingleShot(true);
            connect(&menuClickTimer, &QTimer::timeout, &menuClickTimer,
                    [&sendClick, overflowMenu, expectedAction,
                     &overflowMenuInteractionRan,
                     &overflowMenuVisibleAfterOpen,
                     &overflowActionGeometryValid, &overflowActionAtPoint,
                     &overflowMenuClosedAfterSelection] {
                overflowMenuInteractionRan = true;
                overflowMenuVisibleAfterOpen = overflowMenu->isVisible();
                const QRect actionGeometry = overflowMenu->actionGeometry(
                    expectedAction);
                overflowActionGeometryValid = actionGeometry.isValid()
                                              && !actionGeometry.isEmpty();
                const QPoint actionPoint = actionGeometry.center();
                overflowActionAtPoint = overflowActionGeometryValid
                                        && overflowMenu->actionAt(actionPoint)
                                               == expectedAction;
                if (overflowMenuVisibleAfterOpen && overflowActionAtPoint
                    && expectedAction->isEnabled()) {
                    sendClick(overflowMenu, actionPoint);
                    overflowMenuClosedAfterSelection =
                        !overflowMenu->isVisible();
                }
            });
            menuClickTimer.start(0);
            sendClick(overflowButton, overflowButton->rect().center());
            menuClickTimer.stop();
            overflowMenuClosedAfterSelection =
                overflowMenuClosedAfterSelection || !overflowMenu->isVisible();
        }
        disconnect(triggerConnection);

        const bool directPassed = invocationSurface
                                      == QStringLiteral("directButton")
                                  && sourceButtonVisible && sourceButtonEnabled
                                  && sourceButtonIdentity;
        const bool overflowPassed = invocationSurface
                                        == QStringLiteral(
                                          "collapsedPanelOverflow")
                                    && !sourceButtonVisible && panelCollapsed
                                    && overflowButtonVisible
                                    && overflowButtonEnabled
                                    && overflowActionIdentity
                                    && overflowMenuInteractionRan
                                    && overflowMenuVisibleAfterOpen
                                    && overflowActionGeometryValid
                                    && overflowActionAtPoint
                                    && overflowMenuClosedAfterSelection;

        QJsonObject result;
        result.insert(QStringLiteral("invocationSurface"), invocationSurface);
        result.insert(QStringLiteral("sourceButtonVisible"),
                      sourceButtonVisible);
        result.insert(QStringLiteral("sourceButtonEnabled"),
                      sourceButtonEnabled);
        result.insert(QStringLiteral("sourceButtonIdentity"),
                      sourceButtonIdentity);
        result.insert(QStringLiteral("panelCollapsed"), panelCollapsed);
        result.insert(QStringLiteral("overflowButtonPresent"),
                      overflowButton != nullptr);
        result.insert(QStringLiteral("overflowButtonVisible"),
                      overflowButtonVisible);
        result.insert(QStringLiteral("overflowButtonEnabled"),
                      overflowButtonEnabled);
        result.insert(QStringLiteral("overflowMenuPresent"),
                      overflowMenu != nullptr);
        result.insert(QStringLiteral("overflowActionIdentity"),
                      overflowActionIdentity);
        result.insert(QStringLiteral("overflowMenuInteractionRan"),
                      overflowMenuInteractionRan);
        result.insert(QStringLiteral("overflowMenuVisibleAfterOpen"),
                      overflowMenuVisibleAfterOpen);
        result.insert(QStringLiteral("overflowActionGeometryValid"),
                      overflowActionGeometryValid);
        result.insert(QStringLiteral("overflowActionAtPoint"),
                      overflowActionAtPoint);
        result.insert(QStringLiteral("overflowMenuClosedAfterSelection"),
                      overflowMenuClosedAfterSelection);
        result.insert(QStringLiteral("actionTriggeredByMouse"),
                      actionTriggeredByMouse);
        result.insert(QStringLiteral("passed"),
                      actionTriggeredByMouse && (directPassed || overflowPassed));
        return result;
    };

    auto propertiesStateObject = [](const QVariantMap& state, bool nativeCallback) {
        QJsonObject summary;
        for (const QString& key : {QStringLiteral("document"),
                                   QStringLiteral("currentLayer"),
                                   QStringLiteral("type"),
                                   QStringLiteral("layer"),
                                   QStringLiteral("color"),
                                   QStringLiteral("linetype"),
                                   QStringLiteral("lineweight")}) {
            summary.insert(key, state.value(key).toString());
        }
        QJsonObject result;
        result.insert(QStringLiteral("nativeCallback"), nativeCallback);
        result.insert(QStringLiteral("mode"), state.value("mode").toString());
        result.insert(QStringLiteral("count"), state.value("selectionCount").toInt());
        result.insert(QStringLiteral("summary"), summary);
        return result;
    };

    QJsonObject report;
    report.insert(QStringLiteral("schemaVersion"), 5);
    report.insert(QStringLiteral("product"), qApp->applicationName());
    report.insert(QStringLiteral("version"), qApp->applicationVersion());
    report.insert(QStringLiteral("viewportRequestedWidth"), 1920);
    report.insert(QStringLiteral("viewportRequestedHeight"), 1080);
    report.insert(QStringLiteral("windowWidth"), width());
    report.insert(QStringLiteral("windowHeight"), height());
    const bool sourceDxfLoaded = inputDxfPath.isEmpty()
                                 || (mdi != nullptr && mdi->getDocument() != nullptr
                                     && QFileInfo(mdi->getDocument()->getFilename()).absoluteFilePath()
                                            == QFileInfo(inputDxfPath).absoluteFilePath());
    report.insert(QStringLiteral("sourceDxfLoaded"), sourceDxfLoaded);

    const bool prerequisites = mdi != nullptr && view != nullptr
                               && graphic != nullptr && button != nullptr
                               && button->defaultAction() == a_map.value("DrawLine", nullptr)
                               && polylineButton != nullptr
                               && polylineButton->defaultAction()
                                      == a_map.value("DrawPolyline", nullptr)
                               && modifyDuplicateButton != nullptr
                               && modifyDuplicateButton->defaultAction()
                                      == a_map.value("ModifyDuplicate", nullptr)
                               && modifyMoveButton != nullptr
                               && modifyMoveButton->defaultAction()
                                      == a_map.value("ModifyMove", nullptr)
                               && undoQuickButton != nullptr
                               && undoQuickButton->defaultAction()
                                      == a_map.value("EditUndo", nullptr)
                               && redoQuickButton != nullptr
                               && redoQuickButton->defaultAction()
                                      == a_map.value("EditRedo", nullptr)
                               && kuubikCurrentLayerSelector != nullptr
                               && kuubikPropertiesPalette != nullptr
                               && sourceDxfLoaded
                               && view->width() >= 300 && view->height() >= 200;
    report.insert(QStringLiteral("prerequisites"), prerequisites);
    if (!prerequisites) {
        report.insert(QStringLiteral("status"), QStringLiteral("FAIL"));
        report.insert(QStringLiteral("failure"), QStringLiteral("GUI prerequisites unavailable"));
    } else {
        const QString smokeLayerName = QStringLiteral("KUUBIK-SMOKE-LAYER");
        RS_LayerList* layerList = graphic->getLayerList();
        if (layerList->find(smokeLayerName) == nullptr) {
            graphic->addLayer(new RS_Layer(smokeLayerName));
        }
        const int smokeLayerIndex = kuubikCurrentLayerSelector->findText(smokeLayerName);
        if (smokeLayerIndex >= 0) {
            kuubikCurrentLayerSelector->setCurrentIndex(smokeLayerIndex);
        }
        QApplication::processEvents();

        const QString selectedLayer = kuubikCurrentLayerSelector->currentText();
        const QString nativeCurrentLayer = layerList->getActive() == nullptr
                                               ? QString()
                                               : layerList->getActive()->getName();
        const int entitiesBefore = graphic->getEntityList().size();
        const int linesBefore = lineCount(graphic);
        QSet<RS_Entity*> entitiesBeforeSet;
        for (RS_Entity* entity : graphic->getEntityList()) {
            entitiesBeforeSet.insert(entity);
        }
        const QPoint firstPoint(view->width() / 3, view->height() / 3);
        const QPoint previewPoint(view->width() / 2, view->height() / 2);
        const QPoint secondPoint((view->width() * 2) / 3,
                                 (view->height() * 2) / 3);

        const QString lineRibbonTextBefore = button->text();
        const qint64 lineRibbonIconBefore = button->icon().cacheKey();
        const QJsonObject lineRibbonInvocation = invokeRibbonActionWithMouse(
            button, a_map.value("DrawLine", nullptr));
        const bool lineRibbonMouseEvent = lineRibbonInvocation.value(
            QStringLiteral("passed")).toBool();
        const bool actionActiveAfterRibbon = mdi->getEventHandler() != nullptr
                                             && mdi->getEventHandler()->hasAction();
        const bool lineRibbonPresentationStable =
            lineRibbonTextBefore == QStringLiteral("Line")
            && button->text() == lineRibbonTextBefore
            && button->icon().cacheKey() == lineRibbonIconBefore;
        sendClick(view, firstPoint);

        QMouseEvent move(QEvent::MouseMove, QPointF(previewPoint),
                         Qt::NoButton, Qt::NoButton, Qt::NoModifier);
        QApplication::sendEvent(view, &move);
        QApplication::processEvents();

        QLabel* dynamicInputLabel = view->findChild<QLabel*>(
            QStringLiteral("kuubikDynamicInput"));
        const bool dynamicInputVisible = dynamicInputLabel != nullptr
                                         && dynamicInputLabel->isVisible()
                                         && dynamicInputLabel->text().contains(
                                                QStringLiteral("L "))
                                         && dynamicInputLabel->text().contains(
                                                QStringLiteral("A "));

        const int entitiesAfterFirst = graphic->getEntityList().size();
        const QString activeImagePath = output.filePath(QStringLiteral("line-active.png"));
        const bool activeImageSaved = grab().save(activeImagePath, "PNG");

        sendClick(view, secondPoint);
        const int entitiesAfterSecond = graphic->getEntityList().size();
        const int linesAfterSecond = lineCount(graphic);
        const QString committedImagePath = output.filePath(QStringLiteral("line-committed.png"));
        const bool committedImageSaved = grab().save(committedImagePath, "PNG");

        QKeyEvent lineEnterEvent(QEvent::KeyPress, Qt::Key_Return,
                                 Qt::NoModifier);
        const bool lineEnterAccepted = QApplication::sendEvent(
            view, &lineEnterEvent);
        QApplication::processEvents();
        const bool lineFinishedByEnter = mdi->getEventHandler() != nullptr
                                         && !mdi->getEventHandler()->hasAction();
        if (!lineFinishedByEnter) {
            slotKillAllActions();
            QApplication::processEvents();
        }

        RS_Entity* firstCreatedLine = nullptr;
        for (RS_Entity* entity : graphic->getEntityList()) {
            if (entity != nullptr && !entitiesBeforeSet.contains(entity)
                && entity->rtti() == RS2::EntityLine) {
                firstCreatedLine = entity;
            }
        }
        const QString createdLineLayer = firstCreatedLine == nullptr
                                             || firstCreatedLine->getLayer(true) == nullptr
                                             ? QString()
                                             : firstCreatedLine->getLayer(true)->getName();
        const QString dxfPath = output.filePath(QStringLiteral("line-gui-smoke.dxf"));
        const bool dxfSaved = graphic->saveAs(dxfPath, RS2::FormatDXFRW, true);

        invokeRibbonActionWithMouse(button, a_map.value("DrawLine", nullptr));
        QKeyEvent escapeEvent(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
        QApplication::sendEvent(view, &escapeEvent);
        QApplication::processEvents();
        const bool escapeCancelsAll = escapeEvent.isAccepted()
                                      && mdi->getEventHandler() != nullptr
                                      && !mdi->getEventHandler()->hasAction();

        // Exercise a second native Draw command and the quick-access Undo/Redo
        // buttons through real widget mouse events. The three saved DXFs are
        // independently parsed in CI so the producer's in-memory assertions
        // cannot by themselves make this workflow pass.
        const int activePolylinesBefore = activePolylineCount(graphic);
        QSet<RS_Entity*> entitiesBeforePolylineSet;
        for (RS_Entity* entity : graphic->getEntityList()) {
            entitiesBeforePolylineSet.insert(entity);
        }
        const QPoint polylineFirstPoint(view->width() / 5, view->height() / 4);
        const QPoint polylineSecondPoint(view->width() / 2, view->height() / 5);
        const QPoint polylineThirdPoint((view->width() * 3) / 4,
                                        (view->height() * 2) / 5);

        const QString polylineRibbonTextBefore = polylineButton->text();
        const qint64 polylineRibbonIconBefore = polylineButton->icon().cacheKey();
        const QJsonObject polylineRibbonInvocation = invokeRibbonActionWithMouse(
            polylineButton, a_map.value("DrawPolyline", nullptr));
        const bool polylineActionTriggeredByMouse =
            polylineRibbonInvocation.value(
              QStringLiteral("actionTriggeredByMouse")).toBool();
        RS_ActionInterface* polylineAction = mdi->getEventHandler() == nullptr
                                                 ? nullptr
                                                 : mdi->getEventHandler()
                                                       ->getCurrentAction();
        const bool polylineActionActive = polylineAction != nullptr
                                          && polylineAction->rtti()
                                                 == RS2::ActionDrawPolyline;
        const bool polylineRibbonPresentationStable =
            polylineRibbonTextBefore == QStringLiteral("Polyline")
            && polylineButton->text() == polylineRibbonTextBefore
            && polylineButton->icon().cacheKey() == polylineRibbonIconBefore;
        const int polylineActiveActionType = polylineAction == nullptr
                                                 ? static_cast<int>(RS2::ActionNone)
                                                 : static_cast<int>(
                                                       polylineAction->rtti());
        sendClick(view, polylineFirstPoint);
        sendClick(view, polylineSecondPoint);
        sendClick(view, polylineThirdPoint);
        QKeyEvent polylineEnterEvent(QEvent::KeyPress, Qt::Key_Return,
                                     Qt::NoModifier);
        const bool polylineEnterAccepted = QApplication::sendEvent(
            view, &polylineEnterEvent);
        QApplication::processEvents();
        const bool polylineFinishedByEnter = mdi->getEventHandler() != nullptr
                                             && !mdi->getEventHandler()->hasAction();
        if (!polylineFinishedByEnter) {
            slotKillAllActions();
            QApplication::processEvents();
        }

        RS_Polyline* createdPolyline = nullptr;
        for (RS_Entity* entity : graphic->getEntityList()) {
            if (entity != nullptr && !entitiesBeforePolylineSet.contains(entity)
                && entity->rtti() == RS2::EntityPolyline) {
                createdPolyline = static_cast<RS_Polyline*>(entity);
            }
        }
        const QString createdPolylineLayer = createdPolyline == nullptr
                                                  || createdPolyline->getLayer(true) == nullptr
                                              ? QString()
                                              : createdPolyline->getLayer(true)->getName();
        const int polylineSegmentCount = createdPolyline == nullptr
                                             ? 0
                                             : createdPolyline->count();
        const bool polylineUndoneBeforeUndo = createdPolyline == nullptr
                                              || createdPolyline->isUndone();
        const int activePolylinesBeforeUndo = activePolylineCount(graphic);
        const QString polylineBeforeUndoPath = output.filePath(
            QStringLiteral("pline-before-undo.dxf"));
        const bool polylineBeforeUndoSaved = graphic->saveAs(
            polylineBeforeUndoPath, RS2::FormatDXFRW, true);

        const bool undoNativeIdentity = undoQuickButton->defaultAction()
                                        == a_map.value("EditUndo", nullptr)
                                        && undoQuickButton->property(
                                               "kuubikActionKey").toString()
                                               == QStringLiteral("EditUndo");
        const bool undoQuickAccessButton = undoQuickButton->objectName()
                                           == QStringLiteral("kuubikQuickButton");
        const bool undoVisible = undoQuickButton->isVisible();
        const bool undoEnabledBeforeClick = undoQuickButton->isEnabled();
        bool undoActionTriggeredByMouse = false;
        const QMetaObject::Connection undoConnection = connect(
            a_map.value("EditUndo", nullptr), &QAction::triggered, this,
            [&undoActionTriggeredByMouse](bool) {
                undoActionTriggeredByMouse = true;
            });
        sendClick(undoQuickButton, undoQuickButton->rect().center());
        disconnect(undoConnection);
        QApplication::processEvents();
        const bool polylineUndoneAfterUndo = createdPolyline != nullptr
                                             && createdPolyline->isUndone();
        const bool firstLineActiveAfterUndo = firstCreatedLine != nullptr
                                              && !firstCreatedLine->isUndone();
        const int activePolylinesAfterUndo = activePolylineCount(graphic);
        const QString polylineAfterUndoPath = output.filePath(
            QStringLiteral("pline-after-undo.dxf"));
        const bool polylineAfterUndoSaved = graphic->saveAs(
            polylineAfterUndoPath, RS2::FormatDXFRW, true);

        const bool redoNativeIdentity = redoQuickButton->defaultAction()
                                        == a_map.value("EditRedo", nullptr)
                                        && redoQuickButton->property(
                                               "kuubikActionKey").toString()
                                               == QStringLiteral("EditRedo");
        const bool redoQuickAccessButton = redoQuickButton->objectName()
                                           == QStringLiteral("kuubikQuickButton");
        const bool redoVisible = redoQuickButton->isVisible();
        const bool redoEnabledBeforeClick = redoQuickButton->isEnabled();
        bool redoActionTriggeredByMouse = false;
        const QMetaObject::Connection redoConnection = connect(
            a_map.value("EditRedo", nullptr), &QAction::triggered, this,
            [&redoActionTriggeredByMouse](bool) {
                redoActionTriggeredByMouse = true;
            });
        sendClick(redoQuickButton, redoQuickButton->rect().center());
        disconnect(redoConnection);
        QApplication::processEvents();
        const bool polylineUndoneAfterRedo = createdPolyline == nullptr
                                             || createdPolyline->isUndone();
        const bool firstLineActiveAfterRedo = firstCreatedLine != nullptr
                                              && !firstCreatedLine->isUndone();
        const int activePolylinesAfterRedo = activePolylineCount(graphic);
        const QString polylineAfterRedoPath = output.filePath(
            QStringLiteral("pline-after-redo.dxf"));
        const bool polylineAfterRedoSaved = graphic->saveAs(
            polylineAfterRedoPath, RS2::FormatDXFRW, true);

        const bool polylineRibbonIdentity = polylineButton->defaultAction()
                                            == a_map.value("DrawPolyline", nullptr)
                                            && polylineButton->property(
                                                   "kuubikActionKey").toString()
                                                   == QStringLiteral("DrawPolyline");
        const bool polylinePassed = polylineRibbonIdentity
                                    && polylineRibbonInvocation.value(
                                         QStringLiteral("passed")).toBool()
                                    && polylineActionActive
                                    && polylineRibbonPresentationStable
                                    && polylineEnterAccepted
                                    && polylineFinishedByEnter
                                    && createdPolyline != nullptr
                                    && !createdPolyline->isClosed()
                                    && polylineSegmentCount == 2
                                    && createdPolylineLayer == smokeLayerName
                                    && !polylineUndoneBeforeUndo
                                    && activePolylinesBeforeUndo
                                           == activePolylinesBefore + 1
                                    && undoNativeIdentity && undoQuickAccessButton
                                    && undoVisible && undoEnabledBeforeClick
                                    && undoActionTriggeredByMouse
                                    && polylineUndoneAfterUndo
                                    && firstLineActiveAfterUndo
                                    && activePolylinesAfterUndo
                                           == activePolylinesBefore
                                    && redoNativeIdentity && redoQuickAccessButton
                                    && redoVisible && redoEnabledBeforeClick
                                    && redoActionTriggeredByMouse
                                    && !polylineUndoneAfterRedo
                                    && firstLineActiveAfterRedo
                                    && activePolylinesAfterRedo
                                           == activePolylinesBefore + 1
                                    && polylineBeforeUndoSaved
                                    && polylineAfterUndoSaved
                                    && polylineAfterRedoSaved;

        QJsonObject polylineRibbonObject = polylineRibbonInvocation;
        polylineRibbonObject.insert(QStringLiteral("actionKey"),
                                    QStringLiteral("DrawPolyline"));
        polylineRibbonObject.insert(QStringLiteral("nativeIdentity"),
                                    polylineRibbonIdentity);
        polylineRibbonObject.insert(QStringLiteral("activeActionType"),
                                    polylineActiveActionType);
        polylineRibbonObject.insert(QStringLiteral("nativeActionActive"),
                                    polylineActionActive);
        polylineRibbonObject.insert(QStringLiteral("presentationStable"),
                                    polylineRibbonPresentationStable);

        QJsonObject polylineEntityObject;
        polylineEntityObject.insert(QStringLiteral("created"),
                                    createdPolyline != nullptr);
        polylineEntityObject.insert(QStringLiteral("entityUndoneBeforeUndo"),
                                    polylineUndoneBeforeUndo);
        polylineEntityObject.insert(QStringLiteral("entityUndoneAfterUndo"),
                                    polylineUndoneAfterUndo);
        polylineEntityObject.insert(QStringLiteral("entityUndoneAfterRedo"),
                                    polylineUndoneAfterRedo);
        polylineEntityObject.insert(QStringLiteral("layer"), createdPolylineLayer);
        polylineEntityObject.insert(QStringLiteral("closed"),
                                    createdPolyline != nullptr
                                        && createdPolyline->isClosed());
        polylineEntityObject.insert(QStringLiteral("segmentCount"),
                                    polylineSegmentCount);
        polylineEntityObject.insert(QStringLiteral("verticesExpected"), 3);
        polylineEntityObject.insert(QStringLiteral("activeCountBeforeCreate"),
                                    activePolylinesBefore);
        polylineEntityObject.insert(QStringLiteral("activeCountBeforeUndo"),
                                    activePolylinesBeforeUndo);
        polylineEntityObject.insert(QStringLiteral("activeCountAfterUndo"),
                                    activePolylinesAfterUndo);
        polylineEntityObject.insert(QStringLiteral("activeCountAfterRedo"),
                                    activePolylinesAfterRedo);

        const auto undoRedoActionObject = [](const QString& actionKey,
                                             bool nativeIdentity,
                                             bool quickAccessButton,
                                             bool visible,
                                             bool enabledBeforeClick,
                                             bool actionTriggeredByMouse,
                                             bool firstLineStillActive) {
            QJsonObject object;
            object.insert(QStringLiteral("actionKey"), actionKey);
            object.insert(QStringLiteral("nativeIdentity"), nativeIdentity);
            object.insert(QStringLiteral("quickAccessButton"), quickAccessButton);
            object.insert(QStringLiteral("visible"), visible);
            object.insert(QStringLiteral("enabledBeforeClick"), enabledBeforeClick);
            object.insert(QStringLiteral("actionTriggeredByMouse"),
                          actionTriggeredByMouse);
            object.insert(QStringLiteral("firstLineStillActive"),
                          firstLineStillActive);
            return object;
        };

        QJsonObject polylineFilesObject;
        polylineFilesObject.insert(QStringLiteral("beforeUndo"),
                                   QStringLiteral("pline-before-undo.dxf"));
        polylineFilesObject.insert(QStringLiteral("afterUndo"),
                                   QStringLiteral("pline-after-undo.dxf"));
        polylineFilesObject.insert(QStringLiteral("afterRedo"),
                                   QStringLiteral("pline-after-redo.dxf"));
        polylineFilesObject.insert(QStringLiteral("beforeUndoSaved"),
                                   polylineBeforeUndoSaved);
        polylineFilesObject.insert(QStringLiteral("afterUndoSaved"),
                                   polylineAfterUndoSaved);
        polylineFilesObject.insert(QStringLiteral("afterRedoSaved"),
                                   polylineAfterRedoSaved);

        QJsonObject polylineUndoRedoObject;
        polylineUndoRedoObject.insert(QStringLiteral("ribbon"),
                                      polylineRibbonObject);
        polylineUndoRedoObject.insert(QStringLiteral("polyline"),
                                      polylineEntityObject);
        polylineUndoRedoObject.insert(QStringLiteral("undo"),
                                      undoRedoActionObject(
                                          QStringLiteral("EditUndo"),
                                          undoNativeIdentity,
                                          undoQuickAccessButton, undoVisible,
                                          undoEnabledBeforeClick,
                                          undoActionTriggeredByMouse,
                                          firstLineActiveAfterUndo));
        polylineUndoRedoObject.insert(QStringLiteral("redo"),
                                      undoRedoActionObject(
                                          QStringLiteral("EditRedo"),
                                          redoNativeIdentity,
                                          redoQuickAccessButton, redoVisible,
                                          redoEnabledBeforeClick,
                                          redoActionTriggeredByMouse,
                                          firstLineActiveAfterRedo));
        polylineUndoRedoObject.insert(QStringLiteral("files"),
                                      polylineFilesObject);
        QJsonObject polylineEnterObject;
        polylineEnterObject.insert(QStringLiteral("accepted"),
                                   polylineEnterAccepted);
        polylineEnterObject.insert(QStringLiteral("finishedAction"),
                                   polylineFinishedByEnter);
        polylineUndoRedoObject.insert(QStringLiteral("enter"),
                                      polylineEnterObject);
        polylineUndoRedoObject.insert(QStringLiteral("passed"), polylinePassed);
        report.insert(QStringLiteral("polylineUndoRedo"),
                      polylineUndoRedoObject);

        // Exercise COPY through the existing native ModifyDuplicate QAction.
        // No entity is preselected: the source must be caught by one real
        // canvas click. The default in-place mode makes the clone's geometry
        // independently comparable without introducing a modal offset dialog.
        slotKillAllActions();
        for (RS_Entity* entity : graphic->getEntityList()) {
            if (entity != nullptr) {
                entity->setSelected(false);
            }
        }
        RS_DIALOGFACTORY->updateSelectionWidget(0, 0.0);
        QApplication::processEvents();

        RS_Line* copySourceLine = firstCreatedLine != nullptr
                                      && firstCreatedLine->rtti()
                                             == RS2::EntityLine
                                  ? static_cast<RS_Line*>(firstCreatedLine)
                                  : nullptr;
        const bool copySourceUnselectedBeforeAction = copySourceLine != nullptr
                                                      && !copySourceLine
                                                            ->isSelected()
                                                      && graphic->countSelected()
                                                             == 0;
        const RS_Vector copySourceStart = copySourceLine == nullptr
                                              ? RS_Vector(false)
                                              : copySourceLine->getStartpoint();
        const RS_Vector copySourceEnd = copySourceLine == nullptr
                                            ? RS_Vector(false)
                                            : copySourceLine->getEndpoint();
        const QString copySourceLayer = copySourceLine == nullptr
                                             || copySourceLine->getLayer(true)
                                                    == nullptr
                                            ? QString()
                                            : copySourceLine->getLayer(true)
                                                  ->getName();
        const QString copyActiveLayerBeforeAction = layerList->getActive()
                                                        == nullptr
                                                    ? QString()
                                                    : layerList->getActive()
                                                          ->getName();
        QSet<RS_Entity*> entitiesBeforeCopySet;
        for (RS_Entity* entity : graphic->getEntityList()) {
            entitiesBeforeCopySet.insert(entity);
        }
        const int activeLinesBeforeCopy = activeLineCount(graphic);

        const QJsonObject copyRibbonInvocation = invokeRibbonActionWithMouse(
            modifyDuplicateButton,
            a_map.value("ModifyDuplicate", nullptr));
        RS_ActionInterface* copyActionBase = mdi->getEventHandler() == nullptr
                                                 ? nullptr
                                                 : mdi->getEventHandler()
                                                       ->getCurrentAction();
        const bool copyActionActive = copyActionBase != nullptr
                                      && copyActionBase->rtti()
                                             == RS2::ActionModifyDuplicate;
        auto* copyAction = copyActionActive
                               ? static_cast<LC_ActionModifyDuplicate*>(
                                     copyActionBase)
                               : nullptr;
        // The option normally comes from the user's native Tool Options
        // settings. Pin this automation fixture to in-place COPY so evidence
        // does not depend on a reused profile's last offset preference.
        const bool copyInPlaceForcedForSmoke = copyAction != nullptr;
        if (copyAction != nullptr) {
            copyAction->setDuplicateInPlace(true);
        }
        const bool copyDuplicateInPlace = copyAction != nullptr
                                          && copyAction->isDuplicateInPlace();
        const int copyActiveActionType = copyActionBase == nullptr
                                             ? static_cast<int>(RS2::ActionNone)
                                             : static_cast<int>(
                                                   copyActionBase->rtti());
        const RS_Vector copyCanvasGraphPoint = copySourceLine == nullptr
                                                   ? RS_Vector(false)
                                                   : copySourceStart.lerp(
                                                         copySourceEnd, 0.5);
        const RS_Vector copyCanvasGuiPoint = copyCanvasGraphPoint.valid
                                                 ? view->toGui(
                                                       copyCanvasGraphPoint)
                                                 : RS_Vector(false);
        const QPoint copyCanvasPoint(qRound(copyCanvasGuiPoint.x),
                                     qRound(copyCanvasGuiPoint.y));
        const bool copyCanvasPointInside = copyCanvasGuiPoint.valid
                                           && view->rect().contains(
                                                copyCanvasPoint);
        if (copyActionActive && copyDuplicateInPlace
            && copyCanvasPointInside) {
            sendClick(view, copyCanvasPoint);
        }
        slotKillAllActions();
        QApplication::processEvents();

        RS_Line* copiedLine = nullptr;
        int copyCandidateCount = 0;
        for (RS_Entity* entity : graphic->getEntityList()) {
            if (entity != nullptr && !entitiesBeforeCopySet.contains(entity)
                && entity->rtti() == RS2::EntityLine) {
                copiedLine = static_cast<RS_Line*>(entity);
                ++copyCandidateCount;
            }
        }
        const QString copiedLineLayer = copiedLine == nullptr
                                             || copiedLine->getLayer(true)
                                                    == nullptr
                                            ? QString()
                                            : copiedLine->getLayer(true)
                                                  ->getName();
        const bool copiedLineDistinct = copiedLine != nullptr
                                        && copiedLine != copySourceLine;
        const bool copiedLineStartMatches = copiedLine != nullptr
                                            && copiedLine->getStartpoint()
                                                       .distanceTo(
                                                         copySourceStart)
                                                   <= 1.0e-9;
        const bool copiedLineEndMatches = copiedLine != nullptr
                                          && copiedLine->getEndpoint()
                                                     .distanceTo(copySourceEnd)
                                                 <= 1.0e-9;
        const bool copiedLineUndoneBeforeUndo = copiedLine == nullptr
                                                || copiedLine->isUndone();
        const int activeLinesBeforeCopyUndo = activeLineCount(graphic);
        const QString copyBeforeUndoPath = output.filePath(
            QStringLiteral("copy-before-undo.dxf"));
        const bool copyBeforeUndoSaved = graphic->saveAs(
            copyBeforeUndoPath, RS2::FormatDXFRW, true);

        const bool copyUndoNativeIdentity = undoQuickButton->defaultAction()
                                            == a_map.value("EditUndo", nullptr)
                                            && undoQuickButton->property(
                                                   "kuubikActionKey").toString()
                                                   == QStringLiteral("EditUndo");
        const bool copyUndoQuickAccessButton = undoQuickButton->objectName()
                                               == QStringLiteral(
                                                    "kuubikQuickButton");
        const bool copyUndoVisible = undoQuickButton->isVisible();
        const bool copyUndoEnabledBeforeClick = undoQuickButton->isEnabled();
        bool copyUndoActionTriggeredByMouse = false;
        const QMetaObject::Connection copyUndoConnection = connect(
            a_map.value("EditUndo", nullptr), &QAction::triggered, this,
            [&copyUndoActionTriggeredByMouse](bool) {
                copyUndoActionTriggeredByMouse = true;
            });
        sendClick(undoQuickButton, undoQuickButton->rect().center());
        disconnect(copyUndoConnection);
        QApplication::processEvents();
        const bool copiedLineUndoneAfterUndo = copiedLine != nullptr
                                               && copiedLine->isUndone();
        const bool copySourceActiveAfterUndo = copySourceLine != nullptr
                                               && !copySourceLine->isUndone();
        const bool priorPolylineActiveAfterCopyUndo = createdPolyline != nullptr
                                                      && !createdPolyline
                                                            ->isUndone();
        const int activeLinesAfterCopyUndo = activeLineCount(graphic);
        const QString copyAfterUndoPath = output.filePath(
            QStringLiteral("copy-after-undo.dxf"));
        const bool copyAfterUndoSaved = graphic->saveAs(
            copyAfterUndoPath, RS2::FormatDXFRW, true);

        const bool copyRedoNativeIdentity = redoQuickButton->defaultAction()
                                            == a_map.value("EditRedo", nullptr)
                                            && redoQuickButton->property(
                                                   "kuubikActionKey").toString()
                                                   == QStringLiteral("EditRedo");
        const bool copyRedoQuickAccessButton = redoQuickButton->objectName()
                                               == QStringLiteral(
                                                    "kuubikQuickButton");
        const bool copyRedoVisible = redoQuickButton->isVisible();
        const bool copyRedoEnabledBeforeClick = redoQuickButton->isEnabled();
        bool copyRedoActionTriggeredByMouse = false;
        const QMetaObject::Connection copyRedoConnection = connect(
            a_map.value("EditRedo", nullptr), &QAction::triggered, this,
            [&copyRedoActionTriggeredByMouse](bool) {
                copyRedoActionTriggeredByMouse = true;
            });
        sendClick(redoQuickButton, redoQuickButton->rect().center());
        disconnect(copyRedoConnection);
        QApplication::processEvents();
        const bool copiedLineUndoneAfterRedo = copiedLine == nullptr
                                               || copiedLine->isUndone();
        const bool copySourceActiveAfterRedo = copySourceLine != nullptr
                                               && !copySourceLine->isUndone();
        const bool priorPolylineActiveAfterCopyRedo = createdPolyline != nullptr
                                                      && !createdPolyline
                                                            ->isUndone();
        const int activeLinesAfterCopyRedo = activeLineCount(graphic);
        const QString copyAfterRedoPath = output.filePath(
            QStringLiteral("copy-after-redo.dxf"));
        const bool copyAfterRedoSaved = graphic->saveAs(
            copyAfterRedoPath, RS2::FormatDXFRW, true);

        const bool copyRibbonIdentity = modifyDuplicateButton->defaultAction()
                                        == a_map.value("ModifyDuplicate", nullptr)
                                        && modifyDuplicateButton->property(
                                               "kuubikActionKey").toString()
                                               == QStringLiteral(
                                                    "ModifyDuplicate");
        const bool copyPassed = copyRibbonIdentity
                                && copyRibbonInvocation.value(
                                     QStringLiteral("passed")).toBool()
                                && copyActionActive && copyDuplicateInPlace
                                && copyInPlaceForcedForSmoke
                                && copySourceUnselectedBeforeAction
                                && copyCanvasPointInside
                                && copyCandidateCount == 1
                                && copiedLineDistinct
                                && copyActiveLayerBeforeAction == smokeLayerName
                                && copySourceLayer == smokeLayerName
                                && copiedLineLayer == copySourceLayer
                                && copiedLineStartMatches
                                && copiedLineEndMatches
                                && !copiedLineUndoneBeforeUndo
                                && activeLinesBeforeCopyUndo
                                       == activeLinesBeforeCopy + 1
                                && copyUndoNativeIdentity
                                && copyUndoQuickAccessButton
                                && copyUndoVisible
                                && copyUndoEnabledBeforeClick
                                && copyUndoActionTriggeredByMouse
                                && copiedLineUndoneAfterUndo
                                && copySourceActiveAfterUndo
                                && priorPolylineActiveAfterCopyUndo
                                && activeLinesAfterCopyUndo
                                       == activeLinesBeforeCopy
                                && copyRedoNativeIdentity
                                && copyRedoQuickAccessButton
                                && copyRedoVisible
                                && copyRedoEnabledBeforeClick
                                && copyRedoActionTriggeredByMouse
                                && !copiedLineUndoneAfterRedo
                                && copySourceActiveAfterRedo
                                && priorPolylineActiveAfterCopyRedo
                                && activeLinesAfterCopyRedo
                                       == activeLinesBeforeCopy + 1
                                && copyBeforeUndoSaved
                                && copyAfterUndoSaved
                                && copyAfterRedoSaved;

        QJsonObject copyRibbonObject = copyRibbonInvocation;
        copyRibbonObject.insert(QStringLiteral("actionKey"),
                                QStringLiteral("ModifyDuplicate"));
        copyRibbonObject.insert(QStringLiteral("nativeIdentity"),
                                copyRibbonIdentity);
        copyRibbonObject.insert(QStringLiteral("activeActionType"),
                                copyActiveActionType);
        copyRibbonObject.insert(
            QStringLiteral("expectedActionType"),
            static_cast<int>(RS2::ActionModifyDuplicate));
        copyRibbonObject.insert(QStringLiteral("nativeActionActive"),
                                copyActionActive);

        QJsonObject copiedLineObject;
        copiedLineObject.insert(QStringLiteral("created"), copiedLine != nullptr);
        copiedLineObject.insert(QStringLiteral("candidateCount"),
                                copyCandidateCount);
        copiedLineObject.insert(QStringLiteral("duplicateInPlace"),
                                copyDuplicateInPlace);
        copiedLineObject.insert(QStringLiteral("inPlaceForcedForSmoke"),
                                copyInPlaceForcedForSmoke);
        copiedLineObject.insert(QStringLiteral("sourceUnselectedBeforeAction"),
                                copySourceUnselectedBeforeAction);
        copiedLineObject.insert(QStringLiteral("canvasPointInside"),
                                copyCanvasPointInside);
        copiedLineObject.insert(QStringLiteral("sourceDistinct"),
                                copiedLineDistinct);
        copiedLineObject.insert(QStringLiteral("sourceLayer"),
                                copySourceLayer);
        copiedLineObject.insert(QStringLiteral("activeLayerBeforeAction"),
                                copyActiveLayerBeforeAction);
        copiedLineObject.insert(QStringLiteral("duplicateLayer"),
                                copiedLineLayer);
        copiedLineObject.insert(QStringLiteral("startMatches"),
                                copiedLineStartMatches);
        copiedLineObject.insert(QStringLiteral("endMatches"),
                                copiedLineEndMatches);
        copiedLineObject.insert(QStringLiteral("entityUndoneBeforeUndo"),
                                copiedLineUndoneBeforeUndo);
        copiedLineObject.insert(QStringLiteral("entityUndoneAfterUndo"),
                                copiedLineUndoneAfterUndo);
        copiedLineObject.insert(QStringLiteral("entityUndoneAfterRedo"),
                                copiedLineUndoneAfterRedo);
        copiedLineObject.insert(QStringLiteral("activeCountBeforeCreate"),
                                activeLinesBeforeCopy);
        copiedLineObject.insert(QStringLiteral("activeCountBeforeUndo"),
                                activeLinesBeforeCopyUndo);
        copiedLineObject.insert(QStringLiteral("activeCountAfterUndo"),
                                activeLinesAfterCopyUndo);
        copiedLineObject.insert(QStringLiteral("activeCountAfterRedo"),
                                activeLinesAfterCopyRedo);
        QJsonObject copyCanvasPointObject;
        copyCanvasPointObject.insert(QStringLiteral("graphX"),
                                     copyCanvasGraphPoint.x);
        copyCanvasPointObject.insert(QStringLiteral("graphY"),
                                     copyCanvasGraphPoint.y);
        copyCanvasPointObject.insert(QStringLiteral("guiX"),
                                     copyCanvasPoint.x());
        copyCanvasPointObject.insert(QStringLiteral("guiY"),
                                     copyCanvasPoint.y());
        copyCanvasPointObject.insert(QStringLiteral("inside"),
                                     copyCanvasPointInside);
        copiedLineObject.insert(QStringLiteral("canvasPoint"),
                                copyCanvasPointObject);

        QJsonObject copyUndoObject = undoRedoActionObject(
            QStringLiteral("EditUndo"), copyUndoNativeIdentity,
            copyUndoQuickAccessButton, copyUndoVisible,
            copyUndoEnabledBeforeClick, copyUndoActionTriggeredByMouse,
            copySourceActiveAfterUndo);
        copyUndoObject.insert(QStringLiteral("priorPolylineStillActive"),
                              priorPolylineActiveAfterCopyUndo);
        QJsonObject copyRedoObject = undoRedoActionObject(
            QStringLiteral("EditRedo"), copyRedoNativeIdentity,
            copyRedoQuickAccessButton, copyRedoVisible,
            copyRedoEnabledBeforeClick, copyRedoActionTriggeredByMouse,
            copySourceActiveAfterRedo);
        copyRedoObject.insert(QStringLiteral("priorPolylineStillActive"),
                              priorPolylineActiveAfterCopyRedo);

        QJsonObject copyFilesObject;
        copyFilesObject.insert(QStringLiteral("beforeUndo"),
                               QStringLiteral("copy-before-undo.dxf"));
        copyFilesObject.insert(QStringLiteral("afterUndo"),
                               QStringLiteral("copy-after-undo.dxf"));
        copyFilesObject.insert(QStringLiteral("afterRedo"),
                               QStringLiteral("copy-after-redo.dxf"));
        copyFilesObject.insert(QStringLiteral("beforeUndoSaved"),
                               copyBeforeUndoSaved);
        copyFilesObject.insert(QStringLiteral("afterUndoSaved"),
                               copyAfterUndoSaved);
        copyFilesObject.insert(QStringLiteral("afterRedoSaved"),
                               copyAfterRedoSaved);

        QJsonObject copyUndoRedoObject;
        copyUndoRedoObject.insert(QStringLiteral("ribbon"),
                                  copyRibbonObject);
        copyUndoRedoObject.insert(QStringLiteral("copy"), copiedLineObject);
        copyUndoRedoObject.insert(QStringLiteral("undo"), copyUndoObject);
        copyUndoRedoObject.insert(QStringLiteral("redo"), copyRedoObject);
        copyUndoRedoObject.insert(QStringLiteral("files"), copyFilesObject);
        copyUndoRedoObject.insert(QStringLiteral("passed"), copyPassed);
        report.insert(QStringLiteral("copyUndoRedo"), copyUndoRedoObject);

        const int entitiesBeforePropertyLine = graphic->getEntityList().size();
        const int linesBeforePropertyLine = lineCount(graphic);
        QSet<RS_Entity*> entitiesBeforePropertyLineSet;
        for (RS_Entity* entity : graphic->getEntityList()) {
            entitiesBeforePropertyLineSet.insert(entity);
        }
        const QPoint propertyFirstPoint(view->width() / 4, (view->height() * 2) / 3);
        const QPoint propertySecondPoint(view->width() / 2, (view->height() * 3) / 4);
        const QJsonObject propertiesLineRibbonInvocation =
            invokeRibbonActionWithMouse(button, a_map.value("DrawLine", nullptr));
        sendClick(view, propertyFirstPoint);
        sendClick(view, propertySecondPoint);
        slotKillAllActions();
        QApplication::processEvents();

        RS_Entity* secondCreatedLine = nullptr;
        for (RS_Entity* entity : graphic->getEntityList()) {
            if (entity != nullptr && !entitiesBeforePropertyLineSet.contains(entity)
                && entity->rtti() == RS2::EntityLine) {
                secondCreatedLine = entity;
            }
            if (entity != nullptr) entity->setSelected(false);
        }

        int refreshGeneration = kuubikPropertiesPalette->state()
                                    .value("selectionRefreshGeneration").toInt();
        RS_DIALOGFACTORY->updateSelectionWidget(0, 0.0);
        const QVariantMap documentState = kuubikPropertiesPalette->state();
        const bool documentCallback = documentState.value("selectionRefreshGeneration").toInt()
                                      > refreshGeneration;

        if (firstCreatedLine != nullptr) firstCreatedLine->setSelected(true);
        refreshGeneration = documentState.value("selectionRefreshGeneration").toInt();
        RS_DIALOGFACTORY->updateSelectionWidget(
            graphic->countSelected(), graphic->totalSelectedLength());
        const QVariantMap singleState = kuubikPropertiesPalette->state();
        const bool singleCallback = singleState.value("selectionRefreshGeneration").toInt()
                                    > refreshGeneration;

        QToolButton* fullPropertiesButton = kuubikPropertiesPalette->findChild<QToolButton*>(
            QStringLiteral("kuubikOpenFullProperties"));
        const bool fullPropertiesIdentity = fullPropertiesButton != nullptr
                                            && fullPropertiesButton->defaultAction()
                                                   == a_map.value("ModifyEntity", nullptr);
        if (fullPropertiesButton != nullptr) {
            sendClick(fullPropertiesButton, fullPropertiesButton->rect().center());
        }
        RS_ActionInterface* fullPropertiesAction = mdi->getEventHandler() == nullptr
                                                       ? nullptr
                                                       : mdi->getEventHandler()->getCurrentAction();
        const bool fullPropertiesActionActive = fullPropertiesAction != nullptr
                                                && fullPropertiesAction->rtti()
                                                       == RS2::ActionModifyEntity;
        slotKillAllActions();
        QApplication::processEvents();

        if (firstCreatedLine != nullptr) firstCreatedLine->setSelected(true);
        if (secondCreatedLine != nullptr) secondCreatedLine->setSelected(true);
        refreshGeneration = singleState.value("selectionRefreshGeneration").toInt();
        RS_DIALOGFACTORY->updateSelectionWidget(
            graphic->countSelected(), graphic->totalSelectedLength());
        const QVariantMap multipleState = kuubikPropertiesPalette->state();
        const bool multipleCallback = multipleState.value("selectionRefreshGeneration").toInt()
                                      > refreshGeneration;

        QJsonObject propertiesStates;
        propertiesStates.insert(QStringLiteral("document"),
                                propertiesStateObject(documentState, documentCallback));
        propertiesStates.insert(QStringLiteral("single"),
                                propertiesStateObject(singleState, singleCallback));
        propertiesStates.insert(QStringLiteral("multiple"),
                                propertiesStateObject(multipleState, multipleCallback));
        report.insert(QStringLiteral("propertiesStates"), propertiesStates);
        QJsonObject fullPropertiesActionObject;
        fullPropertiesActionObject.insert(QStringLiteral("actionKey"),
                                          QStringLiteral("ModifyEntity"));
        fullPropertiesActionObject.insert(QStringLiteral("nativeIdentity"),
                                          fullPropertiesIdentity);
        fullPropertiesActionObject.insert(QStringLiteral("nativeActionActive"),
                                          fullPropertiesActionActive);
        report.insert(QStringLiteral("fullPropertiesAction"),
                      fullPropertiesActionObject);
        report.insert(QStringLiteral("propertiesLineRibbonInvocation"),
                      propertiesLineRibbonInvocation);

        QJsonObject layerSelectorObject;
        layerSelectorObject.insert(QStringLiteral("present"), true);
        layerSelectorObject.insert(QStringLiteral("enabled"),
                                   kuubikCurrentLayerSelector->isEnabled());
        layerSelectorObject.insert(QStringLiteral("selectedLayer"), selectedLayer);
        layerSelectorObject.insert(QStringLiteral("nativeCurrentLayer"), nativeCurrentLayer);
        layerSelectorObject.insert(QStringLiteral("createdLineLayer"), createdLineLayer);
        report.insert(QStringLiteral("layerSelector"), layerSelectorObject);

        // Reopen the native DXF through the normal file adapter, switch between
        // both documents, and close the reopened copy. This proves that the
        // selector and Properties palette follow the active MDI document and
        // release their listeners safely when that document is closed.
        slotFileOpen(dxfPath, RS2::FormatDXFRW);
        QApplication::processEvents();
        QC_MDIWindow* reopenedMdi = getMDIWindow();
        RS_Graphic* reopenedGraphic = reopenedMdi == nullptr
                                          ? nullptr
                                          : reopenedMdi->getGraphic();
        const int reopenedLayerIndex = kuubikCurrentLayerSelector->findText(smokeLayerName);
        if (reopenedLayerIndex >= 0) {
            kuubikCurrentLayerSelector->setCurrentIndex(reopenedLayerIndex);
        }
        QApplication::processEvents();
        const bool reopenedBound = reopenedMdi != nullptr && reopenedMdi != mdi
                                   && reopenedGraphic != nullptr
                                   && lineCount(reopenedGraphic) == linesBefore + 1
                                   && reopenedGraphic->getLayerList()->find(smokeLayerName) != nullptr
                                   && kuubikCurrentLayerSelector->layerList()
                                          == reopenedGraphic->getLayerList()
                                   && kuubikCurrentLayerSelector->currentText() == smokeLayerName
                                   && kuubikPropertiesPalette->document()
                                          == reopenedMdi->getDocument();

        doActivate(mdi);
        QApplication::processEvents();
        const bool originalRestored = getMDIWindow() == mdi
                                      && kuubikCurrentLayerSelector->layerList() == layerList
                                      && kuubikCurrentLayerSelector->currentText() == smokeLayerName
                                      && kuubikPropertiesPalette->document() == mdi->getDocument();

        if (reopenedMdi != nullptr && reopenedMdi != mdi) {
            doActivate(reopenedMdi);
            QApplication::processEvents();
            doClose(reopenedMdi, true);
            QApplication::processEvents();
        }
        const bool closeRestoredOriginal = getMDIWindow() == mdi
                                           && kuubikCurrentLayerSelector->layerList() == layerList
                                           && kuubikCurrentLayerSelector->currentText()
                                                  == smokeLayerName
                                           && kuubikPropertiesPalette->document()
                                                  == mdi->getDocument();
        const bool documentLifecyclePassed = reopenedBound
                                             && originalRestored
                                             && closeRestoredOriginal;
        QJsonObject documentLifecycle;
        documentLifecycle.insert(QStringLiteral("reopenedNativeDxf"), reopenedBound);
        documentLifecycle.insert(QStringLiteral("originalRestoredAfterSwitch"),
                                 originalRestored);
        documentLifecycle.insert(QStringLiteral("originalRestoredAfterClose"),
                                 closeRestoredOriginal);
        documentLifecycle.insert(QStringLiteral("passed"), documentLifecyclePassed);
        report.insert(QStringLiteral("documentLifecycle"), documentLifecycle);

        const bool selectorPassed = kuubikCurrentLayerSelector->isEnabled()
                                    && selectedLayer == smokeLayerName
                                    && selectedLayer == nativeCurrentLayer
                                    && selectedLayer == createdLineLayer;
        const bool propertiesPassed = firstCreatedLine != nullptr
                                      && secondCreatedLine != nullptr
                                      && propertiesLineRibbonInvocation.value(
                                           QStringLiteral("passed")).toBool()
                                      && graphic->getEntityList().size()
                                             == entitiesBeforePropertyLine + 1
                                      && lineCount(graphic) == linesBeforePropertyLine + 1
                                      && secondCreatedLine->getLayer(true) != nullptr
                                      && secondCreatedLine->getLayer(true)->getName()
                                             == smokeLayerName
                                      && documentState.value("selectionCount").toInt() == 0
                                      && documentState.value("mode").toString() == "document"
                                      && documentCallback
                                      && singleState.value("selectionCount").toInt() == 1
                                      && singleState.value("mode").toString() == "single"
                                      && singleState.value("type").toString() == tr("Line")
                                      && singleState.value("layer").toString() == smokeLayerName
                                      && singleCallback
                                      && fullPropertiesIdentity
                                      && fullPropertiesActionActive
                                      && multipleState.value("selectionCount").toInt() == 2
                                      && multipleState.value("mode").toString() == "multiple"
                                      && multipleCallback;

        // Exercise MOVE through the native ModifyMove QAction with no
        // preselection. One canvas click selects the unique Properties-smoke
        // LINE, Enter advances LibreCAD's selection action, two more canvas
        // clicks provide the reference and target points, and the native move
        // dialog is completed through real widget mouse events.
        slotKillAllActions();
        QApplication::processEvents();
        RS_Line* moveSourceLine = secondCreatedLine != nullptr
                                          && secondCreatedLine->rtti()
                                                 == RS2::EntityLine
                                      ? static_cast<RS_Line*>(secondCreatedLine)
                                      : nullptr;
        const bool moveSourceUnselectedBeforeAction = moveSourceLine != nullptr
                                                      && !moveSourceLine
                                                            ->isSelected()
                                                      && graphic->countSelected()
                                                             == 0;
        const RS_Vector moveSourceStart = moveSourceLine == nullptr
                                              ? RS_Vector(false)
                                              : moveSourceLine->getStartpoint();
        const RS_Vector moveSourceEnd = moveSourceLine == nullptr
                                            ? RS_Vector(false)
                                            : moveSourceLine->getEndpoint();
        const QString moveSourceLayer = moveSourceLine == nullptr
                                                || moveSourceLine->getLayer(true)
                                                       == nullptr
                                            ? QString()
                                            : moveSourceLine->getLayer(true)
                                                  ->getName();
        const QString moveActiveLayerBeforeAction = layerList->getActive()
                                                        == nullptr
                                                    ? QString()
                                                    : layerList->getActive()
                                                          ->getName();
        QSet<RS_Entity*> entitiesBeforeMoveSet;
        for (RS_Entity* entity : graphic->getEntityList()) {
            entitiesBeforeMoveSet.insert(entity);
        }
        const int activeLinesBeforeMove = activeLineCount(graphic);

        const RS_SnapMode moveSavedSnapMode = view->getDefaultSnapMode();
        RS_SnapMode moveSmokeSnapMode = moveSavedSnapMode;
        moveSmokeSnapMode.clear();
        view->setDefaultSnapMode(moveSmokeSnapMode);
        const bool moveSnapModeTemporarilyCleared =
            view->getDefaultSnapMode() == moveSmokeSnapMode;

        const QJsonObject moveRibbonInvocation = invokeRibbonActionWithMouse(
            modifyMoveButton, a_map.value("ModifyMove", nullptr));
        RS_ActionInterface* moveSelectionAction = mdi->getEventHandler() == nullptr
                                                      ? nullptr
                                                      : mdi->getEventHandler()
                                                            ->getCurrentAction();
        const int moveInitialActionType = moveSelectionAction == nullptr
                                              ? static_cast<int>(RS2::ActionNone)
                                              : static_cast<int>(
                                                    moveSelectionAction->rtti());
        const bool moveSelectionActionActive = moveSelectionAction != nullptr
                                               && moveSelectionAction->rtti()
                                                      == RS2::ActionSelectSingle;
        const RS_Vector moveSelectionGraphPoint = moveSourceLine == nullptr
                                                      ? RS_Vector(false)
                                                      : moveSourceStart.lerp(
                                                            moveSourceEnd, 0.5);
        const RS_Vector moveSelectionGuiPoint = moveSelectionGraphPoint.valid
                                                    ? view->toGui(
                                                          moveSelectionGraphPoint)
                                                    : RS_Vector(false);
        const QPoint moveSelectionPoint(qRound(moveSelectionGuiPoint.x),
                                        qRound(moveSelectionGuiPoint.y));
        const bool moveSelectionPointInside = moveSelectionGuiPoint.valid
                                              && view->rect().contains(
                                                   moveSelectionPoint);
        if (moveSelectionActionActive && moveSelectionPointInside) {
            sendClick(view, moveSelectionPoint);
        }
        const bool moveSourceSelectedByCanvas = moveSourceLine != nullptr
                                                && moveSourceLine->isSelected()
                                                && graphic->countSelected() == 1;
        if (moveSourceSelectedByCanvas) {
            QKeyEvent moveEnterPress(QEvent::KeyPress, Qt::Key_Enter,
                                     Qt::NoModifier);
            QApplication::sendEvent(view, &moveEnterPress);
            QApplication::processEvents();
        }
        RS_ActionInterface* moveActionAfterSelection =
            mdi->getEventHandler() == nullptr
                ? nullptr
                : mdi->getEventHandler()->getCurrentAction();
        const int moveActionTypeAfterSelection = moveActionAfterSelection == nullptr
                                                     ? static_cast<int>(
                                                           RS2::ActionNone)
                                                     : static_cast<int>(
                                                           moveActionAfterSelection
                                                               ->rtti());
        const bool moveNativeActionActive = moveActionAfterSelection != nullptr
                                            && moveActionAfterSelection->rtti()
                                                   == RS2::ActionModifyMove;

        const RS_Vector moveReferenceGuiPoint = moveSourceStart.valid
                                                    ? view->toGui(moveSourceStart)
                                                    : RS_Vector(false);
        const QPoint moveReferencePoint(qRound(moveReferenceGuiPoint.x),
                                        qRound(moveReferenceGuiPoint.y));
        const QPoint moveTargetPoint(moveReferencePoint.x() + 96,
                                     moveReferencePoint.y() - 72);
        const bool moveReferencePointInside = moveReferenceGuiPoint.valid
                                              && view->rect().contains(
                                                   moveReferencePoint);
        const bool moveTargetPointInside = view->rect().contains(moveTargetPoint)
                                           && moveTargetPoint
                                                  != moveReferencePoint;
        if (moveNativeActionActive && moveReferencePointInside
            && moveTargetPointInside) {
            sendClick(view, moveReferencePoint);
        }
        RS_ActionInterface* moveActionAfterReference =
            mdi->getEventHandler() == nullptr
                ? nullptr
                : mdi->getEventHandler()->getCurrentAction();
        const bool moveActionActiveAfterReference =
            moveActionAfterReference != nullptr
            && moveActionAfterReference->rtti() == RS2::ActionModifyMove;

        bool moveDialogTimerRan = false;
        bool moveDialogFound = false;
        bool moveDialogVisible = false;
        bool moveModeControlFound = false;
        bool moveModeClickedByMouse = false;
        bool moveModeSelected = false;
        bool moveDialogOkFound = false;
        bool moveDialogOkClickedByMouse = false;
        bool moveDialogAcceptedByMouse = false;
        bool moveDialogSafetyTriggered = false;
        QTimer moveDialogTimer;
        moveDialogTimer.setSingleShot(true);
        connect(&moveDialogTimer, &QTimer::timeout, &moveDialogTimer,
                [&sendClick, &moveDialogTimerRan, &moveDialogFound,
                 &moveDialogVisible, &moveModeControlFound,
                 &moveModeClickedByMouse, &moveModeSelected,
                 &moveDialogOkFound, &moveDialogOkClickedByMouse,
                 &moveDialogAcceptedByMouse] {
            moveDialogTimerRan = true;
            QWidget* dialog = QApplication::activeModalWidget();
            moveDialogFound = dialog != nullptr
                              && dialog->objectName()
                                     == QStringLiteral("QG_DlgMove");
            moveDialogVisible = moveDialogFound && dialog->isVisible();
            if (!moveDialogVisible) return;

            auto* moveMode = dialog->findChild<QRadioButton*>(
                QStringLiteral("rbMove"));
            auto* buttonBox = dialog->findChild<QDialogButtonBox*>(
                QStringLiteral("buttonBox"));
            moveModeControlFound = moveMode != nullptr
                                   && moveMode->isVisible()
                                   && moveMode->isEnabled();
            if (moveModeControlFound) {
                sendClick(moveMode, moveMode->rect().center());
                moveModeClickedByMouse = true;
                moveModeSelected = moveMode->isChecked();
            }

            QAbstractButton* okButton = nullptr;
            if (buttonBox != nullptr) {
                for (QAbstractButton* candidate : buttonBox->buttons()) {
                    if (buttonBox->standardButton(candidate)
                        == QDialogButtonBox::Ok) {
                        okButton = candidate;
                        break;
                    }
                }
            }
            moveDialogOkFound = okButton != nullptr
                                && okButton->isVisible()
                                && okButton->isEnabled();
            if (moveModeSelected && moveDialogOkFound) {
                sendClick(okButton, okButton->rect().center());
                moveDialogOkClickedByMouse = true;
                moveDialogAcceptedByMouse = !dialog->isVisible();
            }
        });
        QTimer moveDialogSafetyTimer;
        moveDialogSafetyTimer.setSingleShot(true);
        connect(&moveDialogSafetyTimer, &QTimer::timeout,
                &moveDialogSafetyTimer, [&sendClick,
                                         &moveDialogSafetyTriggered] {
            QWidget* dialog = QApplication::activeModalWidget();
            if (dialog == nullptr
                || dialog->objectName() != QStringLiteral("QG_DlgMove")
                || !dialog->isVisible()) {
                return;
            }
            moveDialogSafetyTriggered = true;
            auto* buttonBox = dialog->findChild<QDialogButtonBox*>(
                QStringLiteral("buttonBox"));
            if (buttonBox != nullptr) {
                for (QAbstractButton* candidate : buttonBox->buttons()) {
                    if (buttonBox->standardButton(candidate)
                        == QDialogButtonBox::Cancel) {
                        sendClick(candidate, candidate->rect().center());
                        return;
                    }
                }
            }
            // This branch is only a hang-prevention fallback. The smoke fails
            // whenever the safety timer fires, so a direct reject can never
            // turn a missing native mouse path into passing evidence.
            QMetaObject::invokeMethod(dialog, "reject", Qt::DirectConnection);
        });
        if (moveActionActiveAfterReference && moveTargetPointInside) {
            moveDialogTimer.start(0);
            moveDialogSafetyTimer.start(2000);
            sendClick(view, moveTargetPoint);
            moveDialogTimer.stop();
            moveDialogSafetyTimer.stop();
        }
        view->setDefaultSnapMode(moveSavedSnapMode);
        const bool moveSnapModeRestored =
            view->getDefaultSnapMode() == moveSavedSnapMode;
        slotKillAllActions();
        QApplication::processEvents();

        RS_Line* movedLine = nullptr;
        int moveCandidateCount = 0;
        for (RS_Entity* entity : graphic->getEntityList()) {
            if (entity != nullptr && !entitiesBeforeMoveSet.contains(entity)
                && entity->rtti() == RS2::EntityLine) {
                movedLine = static_cast<RS_Line*>(entity);
                ++moveCandidateCount;
            }
        }
        const QString movedLineLayer = movedLine == nullptr
                                               || movedLine->getLayer(true)
                                                      == nullptr
                                           ? QString()
                                           : movedLine->getLayer(true)
                                                 ->getName();
        const RS_Vector moveOffsetFromStart = movedLine == nullptr
                                                  ? RS_Vector(false)
                                                  : movedLine->getStartpoint()
                                                        - moveSourceStart;
        const RS_Vector moveOffsetFromEnd = movedLine == nullptr
                                                ? RS_Vector(false)
                                                : movedLine->getEndpoint()
                                                      - moveSourceEnd;
        const bool moveOffsetNonZero = moveOffsetFromStart.valid
                                       && moveOffsetFromStart.magnitude()
                                              > 1.0e-6;
        const bool moveOffsetMatches = moveOffsetFromStart.valid
                                       && moveOffsetFromEnd.valid
                                       && moveOffsetFromStart.distanceTo(
                                            moveOffsetFromEnd)
                                              <= 1.0e-9;
        const bool moveSourceUndoneBeforeUndo = moveSourceLine != nullptr
                                                && moveSourceLine->isUndone();
        const bool movedLineUndoneBeforeUndo = movedLine == nullptr
                                               || movedLine->isUndone();
        const int activeLinesBeforeMoveUndo = activeLineCount(graphic);
        const bool moveReadyForUndo = moveCandidateCount == 1
                                      && movedLine != nullptr
                                      && movedLine != moveSourceLine
                                      && moveSourceUndoneBeforeUndo
                                      && !movedLineUndoneBeforeUndo
                                      && moveOffsetNonZero
                                      && moveOffsetMatches
                                      && activeLinesBeforeMoveUndo
                                             == activeLinesBeforeMove;
        const QString moveBeforeUndoPath = output.filePath(
            QStringLiteral("move-before-undo.dxf"));
        const bool moveBeforeUndoSaved = graphic->saveAs(
            moveBeforeUndoPath, RS2::FormatDXFRW, true);

        const bool moveUndoNativeIdentity = undoQuickButton->defaultAction()
                                            == a_map.value("EditUndo", nullptr)
                                            && undoQuickButton->property(
                                                   "kuubikActionKey").toString()
                                                   == QStringLiteral("EditUndo");
        const bool moveUndoQuickAccessButton = undoQuickButton->objectName()
                                               == QStringLiteral(
                                                    "kuubikQuickButton");
        const bool moveUndoVisible = undoQuickButton->isVisible();
        const bool moveUndoEnabledBeforeClick = undoQuickButton->isEnabled();
        bool moveUndoActionTriggeredByMouse = false;
        const QMetaObject::Connection moveUndoConnection = connect(
            a_map.value("EditUndo", nullptr), &QAction::triggered, this,
            [&moveUndoActionTriggeredByMouse](bool) {
                moveUndoActionTriggeredByMouse = true;
            });
        if (moveReadyForUndo && moveUndoEnabledBeforeClick) {
            sendClick(undoQuickButton, undoQuickButton->rect().center());
        }
        disconnect(moveUndoConnection);
        QApplication::processEvents();
        const bool moveSourceActiveAfterUndo = moveSourceLine != nullptr
                                               && !moveSourceLine->isUndone();
        const bool movedLineUndoneAfterUndo = movedLine != nullptr
                                              && movedLine->isUndone();
        const bool firstLineActiveAfterMoveUndo = firstCreatedLine != nullptr
                                                  && !firstCreatedLine
                                                        ->isUndone();
        const bool copyActiveAfterMoveUndo = copiedLine != nullptr
                                             && !copiedLine->isUndone();
        const bool priorPolylineActiveAfterMoveUndo = createdPolyline != nullptr
                                                      && !createdPolyline
                                                            ->isUndone();
        const int activeLinesAfterMoveUndo = activeLineCount(graphic);
        const QString moveAfterUndoPath = output.filePath(
            QStringLiteral("move-after-undo.dxf"));
        const bool moveAfterUndoSaved = graphic->saveAs(
            moveAfterUndoPath, RS2::FormatDXFRW, true);

        const bool moveRedoNativeIdentity = redoQuickButton->defaultAction()
                                            == a_map.value("EditRedo", nullptr)
                                            && redoQuickButton->property(
                                                   "kuubikActionKey").toString()
                                                   == QStringLiteral("EditRedo");
        const bool moveRedoQuickAccessButton = redoQuickButton->objectName()
                                               == QStringLiteral(
                                                    "kuubikQuickButton");
        const bool moveRedoVisible = redoQuickButton->isVisible();
        const bool moveRedoEnabledBeforeClick = redoQuickButton->isEnabled();
        bool moveRedoActionTriggeredByMouse = false;
        const QMetaObject::Connection moveRedoConnection = connect(
            a_map.value("EditRedo", nullptr), &QAction::triggered, this,
            [&moveRedoActionTriggeredByMouse](bool) {
                moveRedoActionTriggeredByMouse = true;
            });
        if (moveSourceActiveAfterUndo && movedLineUndoneAfterUndo
            && moveRedoEnabledBeforeClick) {
            sendClick(redoQuickButton, redoQuickButton->rect().center());
        }
        disconnect(moveRedoConnection);
        QApplication::processEvents();
        const bool moveSourceUndoneAfterRedo = moveSourceLine != nullptr
                                               && moveSourceLine->isUndone();
        const bool movedLineActiveAfterRedo = movedLine != nullptr
                                              && !movedLine->isUndone();
        const bool firstLineActiveAfterMoveRedo = firstCreatedLine != nullptr
                                                  && !firstCreatedLine
                                                        ->isUndone();
        const bool copyActiveAfterMoveRedo = copiedLine != nullptr
                                             && !copiedLine->isUndone();
        const bool priorPolylineActiveAfterMoveRedo = createdPolyline != nullptr
                                                      && !createdPolyline
                                                            ->isUndone();
        const int activeLinesAfterMoveRedo = activeLineCount(graphic);
        const QString moveAfterRedoPath = output.filePath(
            QStringLiteral("move-after-redo.dxf"));
        const bool moveAfterRedoSaved = graphic->saveAs(
            moveAfterRedoPath, RS2::FormatDXFRW, true);

        const bool moveRibbonIdentity = modifyMoveButton->defaultAction()
                                        == a_map.value("ModifyMove", nullptr)
                                        && modifyMoveButton->property(
                                               "kuubikActionKey").toString()
                                               == QStringLiteral("ModifyMove");
        const bool movePassed = moveRibbonIdentity
                                && moveRibbonInvocation.value(
                                     QStringLiteral("passed")).toBool()
                                && moveSourceUnselectedBeforeAction
                                && moveSelectionActionActive
                                && moveSelectionPointInside
                                && moveSourceSelectedByCanvas
                                && moveNativeActionActive
                                && moveReferencePointInside
                                && moveTargetPointInside
                                && moveActionActiveAfterReference
                                && moveDialogTimerRan && moveDialogFound
                                && moveDialogVisible && moveModeControlFound
                                && moveModeClickedByMouse && moveModeSelected
                                && moveDialogOkFound
                                && moveDialogOkClickedByMouse
                                && moveDialogAcceptedByMouse
                                && !moveDialogSafetyTriggered
                                && moveSnapModeTemporarilyCleared
                                && moveSnapModeRestored
                                && moveCandidateCount == 1
                                && movedLine != moveSourceLine
                                && moveSourceLayer == smokeLayerName
                                && moveActiveLayerBeforeAction == smokeLayerName
                                && movedLineLayer == moveSourceLayer
                                && moveOffsetNonZero && moveOffsetMatches
                                && moveSourceUndoneBeforeUndo
                                && !movedLineUndoneBeforeUndo
                                && activeLinesBeforeMoveUndo
                                       == activeLinesBeforeMove
                                && moveUndoNativeIdentity
                                && moveUndoQuickAccessButton
                                && moveUndoVisible
                                && moveUndoEnabledBeforeClick
                                && moveUndoActionTriggeredByMouse
                                && moveSourceActiveAfterUndo
                                && movedLineUndoneAfterUndo
                                && firstLineActiveAfterMoveUndo
                                && copyActiveAfterMoveUndo
                                && priorPolylineActiveAfterMoveUndo
                                && activeLinesAfterMoveUndo
                                       == activeLinesBeforeMove
                                && moveRedoNativeIdentity
                                && moveRedoQuickAccessButton
                                && moveRedoVisible
                                && moveRedoEnabledBeforeClick
                                && moveRedoActionTriggeredByMouse
                                && moveSourceUndoneAfterRedo
                                && movedLineActiveAfterRedo
                                && firstLineActiveAfterMoveRedo
                                && copyActiveAfterMoveRedo
                                && priorPolylineActiveAfterMoveRedo
                                && activeLinesAfterMoveRedo
                                       == activeLinesBeforeMove
                                && moveBeforeUndoSaved
                                && moveAfterUndoSaved
                                && moveAfterRedoSaved;

        QJsonObject moveRibbonObject = moveRibbonInvocation;
        moveRibbonObject.insert(QStringLiteral("actionKey"),
                                QStringLiteral("ModifyMove"));
        moveRibbonObject.insert(QStringLiteral("nativeIdentity"),
                                moveRibbonIdentity);
        moveRibbonObject.insert(QStringLiteral("selectionActionActive"),
                                moveSelectionActionActive);
        moveRibbonObject.insert(QStringLiteral("initialActionType"),
                                moveInitialActionType);
        moveRibbonObject.insert(QStringLiteral("expectedSelectionActionType"),
                                static_cast<int>(RS2::ActionSelectSingle));
        moveRibbonObject.insert(QStringLiteral("nativeActionActive"),
                                moveNativeActionActive);
        moveRibbonObject.insert(QStringLiteral("actionTypeAfterSelection"),
                                moveActionTypeAfterSelection);
        moveRibbonObject.insert(QStringLiteral("expectedMoveActionType"),
                                static_cast<int>(RS2::ActionModifyMove));

        auto vectorObject = [](const RS_Vector& vector) {
            QJsonObject object;
            object.insert(QStringLiteral("valid"), vector.valid);
            object.insert(QStringLiteral("x"), vector.x);
            object.insert(QStringLiteral("y"), vector.y);
            return object;
        };
        auto guiPointObject = [&view](const QPoint& point) {
            QJsonObject object;
            object.insert(QStringLiteral("x"), point.x());
            object.insert(QStringLiteral("y"), point.y());
            object.insert(QStringLiteral("inside"),
                          view->rect().contains(point));
            return object;
        };
        QJsonObject moveEntityObject;
        moveEntityObject.insert(QStringLiteral("created"), movedLine != nullptr);
        moveEntityObject.insert(QStringLiteral("candidateCount"),
                                moveCandidateCount);
        moveEntityObject.insert(QStringLiteral("sourceUnselectedBeforeAction"),
                                moveSourceUnselectedBeforeAction);
        moveEntityObject.insert(QStringLiteral("sourceSelectedByCanvas"),
                                moveSourceSelectedByCanvas);
        moveEntityObject.insert(QStringLiteral("sourceLayer"), moveSourceLayer);
        moveEntityObject.insert(QStringLiteral("activeLayerBeforeAction"),
                                moveActiveLayerBeforeAction);
        moveEntityObject.insert(QStringLiteral("movedLayer"), movedLineLayer);
        moveEntityObject.insert(QStringLiteral("sourceStart"),
                                vectorObject(moveSourceStart));
        moveEntityObject.insert(QStringLiteral("sourceEnd"),
                                vectorObject(moveSourceEnd));
        moveEntityObject.insert(QStringLiteral("movedStart"),
                                vectorObject(movedLine == nullptr
                                                 ? RS_Vector(false)
                                                 : movedLine->getStartpoint()));
        moveEntityObject.insert(QStringLiteral("movedEnd"),
                                vectorObject(movedLine == nullptr
                                                 ? RS_Vector(false)
                                                 : movedLine->getEndpoint()));
        moveEntityObject.insert(QStringLiteral("offset"),
                                vectorObject(moveOffsetFromStart));
        moveEntityObject.insert(QStringLiteral("offsetNonZero"),
                                moveOffsetNonZero);
        moveEntityObject.insert(QStringLiteral("offsetMatchesBothEndpoints"),
                                moveOffsetMatches);
        moveEntityObject.insert(QStringLiteral("selectionCanvasPoint"),
                                guiPointObject(moveSelectionPoint));
        moveEntityObject.insert(QStringLiteral("referenceCanvasPoint"),
                                guiPointObject(moveReferencePoint));
        moveEntityObject.insert(QStringLiteral("targetCanvasPoint"),
                                guiPointObject(moveTargetPoint));
        moveEntityObject.insert(QStringLiteral("sourceUndoneBeforeUndo"),
                                moveSourceUndoneBeforeUndo);
        moveEntityObject.insert(QStringLiteral("movedUndoneBeforeUndo"),
                                movedLineUndoneBeforeUndo);
        moveEntityObject.insert(QStringLiteral("sourceActiveAfterUndo"),
                                moveSourceActiveAfterUndo);
        moveEntityObject.insert(QStringLiteral("movedUndoneAfterUndo"),
                                movedLineUndoneAfterUndo);
        moveEntityObject.insert(QStringLiteral("sourceUndoneAfterRedo"),
                                moveSourceUndoneAfterRedo);
        moveEntityObject.insert(QStringLiteral("movedActiveAfterRedo"),
                                movedLineActiveAfterRedo);
        moveEntityObject.insert(QStringLiteral("activeCountBeforeMove"),
                                activeLinesBeforeMove);
        moveEntityObject.insert(QStringLiteral("activeCountBeforeUndo"),
                                activeLinesBeforeMoveUndo);
        moveEntityObject.insert(QStringLiteral("activeCountAfterUndo"),
                                activeLinesAfterMoveUndo);
        moveEntityObject.insert(QStringLiteral("activeCountAfterRedo"),
                                activeLinesAfterMoveRedo);
        moveEntityObject.insert(QStringLiteral("snapModeTemporarilyCleared"),
                                moveSnapModeTemporarilyCleared);
        moveEntityObject.insert(QStringLiteral("snapModeRestored"),
                                moveSnapModeRestored);

        QJsonObject moveDialogObject;
        moveDialogObject.insert(QStringLiteral("objectName"),
                                QStringLiteral("QG_DlgMove"));
        moveDialogObject.insert(QStringLiteral("timerRan"),
                                moveDialogTimerRan);
        moveDialogObject.insert(QStringLiteral("found"), moveDialogFound);
        moveDialogObject.insert(QStringLiteral("visible"), moveDialogVisible);
        moveDialogObject.insert(QStringLiteral("moveModeControlFound"),
                                moveModeControlFound);
        moveDialogObject.insert(QStringLiteral("moveModeClickedByMouse"),
                                moveModeClickedByMouse);
        moveDialogObject.insert(QStringLiteral("moveModeSelected"),
                                moveModeSelected);
        moveDialogObject.insert(QStringLiteral("okFound"), moveDialogOkFound);
        moveDialogObject.insert(QStringLiteral("okClickedByMouse"),
                                moveDialogOkClickedByMouse);
        moveDialogObject.insert(QStringLiteral("acceptedByMouse"),
                                moveDialogAcceptedByMouse);
        moveDialogObject.insert(QStringLiteral("safetyTriggered"),
                                moveDialogSafetyTriggered);

        QJsonObject moveUndoObject = undoRedoActionObject(
            QStringLiteral("EditUndo"), moveUndoNativeIdentity,
            moveUndoQuickAccessButton, moveUndoVisible,
            moveUndoEnabledBeforeClick, moveUndoActionTriggeredByMouse,
            firstLineActiveAfterMoveUndo);
        moveUndoObject.insert(QStringLiteral("copyStillActive"),
                              copyActiveAfterMoveUndo);
        moveUndoObject.insert(QStringLiteral("priorPolylineStillActive"),
                              priorPolylineActiveAfterMoveUndo);
        QJsonObject moveRedoObject = undoRedoActionObject(
            QStringLiteral("EditRedo"), moveRedoNativeIdentity,
            moveRedoQuickAccessButton, moveRedoVisible,
            moveRedoEnabledBeforeClick, moveRedoActionTriggeredByMouse,
            firstLineActiveAfterMoveRedo);
        moveRedoObject.insert(QStringLiteral("copyStillActive"),
                              copyActiveAfterMoveRedo);
        moveRedoObject.insert(QStringLiteral("priorPolylineStillActive"),
                              priorPolylineActiveAfterMoveRedo);

        QJsonObject moveFilesObject;
        moveFilesObject.insert(QStringLiteral("beforeUndo"),
                               QStringLiteral("move-before-undo.dxf"));
        moveFilesObject.insert(QStringLiteral("afterUndo"),
                               QStringLiteral("move-after-undo.dxf"));
        moveFilesObject.insert(QStringLiteral("afterRedo"),
                               QStringLiteral("move-after-redo.dxf"));
        moveFilesObject.insert(QStringLiteral("beforeUndoSaved"),
                               moveBeforeUndoSaved);
        moveFilesObject.insert(QStringLiteral("afterUndoSaved"),
                               moveAfterUndoSaved);
        moveFilesObject.insert(QStringLiteral("afterRedoSaved"),
                               moveAfterRedoSaved);

        QJsonObject moveUndoRedoObject;
        moveUndoRedoObject.insert(QStringLiteral("ribbon"), moveRibbonObject);
        moveUndoRedoObject.insert(QStringLiteral("move"), moveEntityObject);
        moveUndoRedoObject.insert(QStringLiteral("dialog"), moveDialogObject);
        moveUndoRedoObject.insert(QStringLiteral("undo"), moveUndoObject);
        moveUndoRedoObject.insert(QStringLiteral("redo"), moveRedoObject);
        moveUndoRedoObject.insert(QStringLiteral("files"), moveFilesObject);
        moveUndoRedoObject.insert(QStringLiteral("passed"), movePassed);
        report.insert(QStringLiteral("moveUndoRedo"), moveUndoRedoObject);

        const bool passed = lineRibbonMouseEvent && actionActiveAfterRibbon
                            && lineRibbonPresentationStable
                            && lineEnterAccepted && lineFinishedByEnter
                            && entitiesAfterFirst == entitiesBefore
                            && dynamicInputVisible
                            && escapeCancelsAll
                            && entitiesAfterSecond == entitiesBefore + 1
                            && linesAfterSecond == linesBefore + 1
                            && activeImageSaved && committedImageSaved && dxfSaved
                            && selectorPassed && propertiesPassed
                            && documentLifecyclePassed && polylinePassed
                            && copyPassed && movePassed;

        report.insert(QStringLiteral("ribbonActionKey"), QStringLiteral("DrawLine"));
        report.insert(QStringLiteral("ribbonMouseEvent"), lineRibbonMouseEvent);
        report.insert(QStringLiteral("ribbonInvocation"), lineRibbonInvocation);
        report.insert(QStringLiteral("actionActiveAfterRibbon"), actionActiveAfterRibbon);
        report.insert(QStringLiteral("lineRibbonPresentationStable"),
                      lineRibbonPresentationStable);
        QJsonObject lineEnterObject;
        lineEnterObject.insert(QStringLiteral("accepted"), lineEnterAccepted);
        lineEnterObject.insert(QStringLiteral("finishedAction"),
                               lineFinishedByEnter);
        report.insert(QStringLiteral("lineEnter"), lineEnterObject);
        report.insert(QStringLiteral("entitiesBefore"), entitiesBefore);
        report.insert(QStringLiteral("entitiesAfterFirstClick"), entitiesAfterFirst);
        report.insert(QStringLiteral("dynamicInputVisible"), dynamicInputVisible);
        report.insert(QStringLiteral("escapeCancelsAll"), escapeCancelsAll);
        report.insert(QStringLiteral("entitiesAfterSecondClick"), entitiesAfterSecond);
        report.insert(QStringLiteral("linesBefore"), linesBefore);
        report.insert(QStringLiteral("linesAfterSecondClick"), linesAfterSecond);
        report.insert(QStringLiteral("activeImageSaved"), activeImageSaved);
        report.insert(QStringLiteral("committedImageSaved"), committedImageSaved);
        report.insert(QStringLiteral("dxfSaved"), dxfSaved);
        report.insert(QStringLiteral("selectorPassed"), selectorPassed);
        report.insert(QStringLiteral("propertiesPassed"), propertiesPassed);
        report.insert(QStringLiteral("status"), passed ? QStringLiteral("PASS")
                                                       : QStringLiteral("FAIL"));

        auto pointObject = [](const QPoint& point) {
            QJsonObject result;
            result.insert(QStringLiteral("x"), point.x());
            result.insert(QStringLiteral("y"), point.y());
            return result;
        };
        report.insert(QStringLiteral("firstCanvasPoint"), pointObject(firstPoint));
        report.insert(QStringLiteral("previewCanvasPoint"), pointObject(previewPoint));
        report.insert(QStringLiteral("secondCanvasPoint"), pointObject(secondPoint));
    }

    const QString reportPath = output.filePath(QStringLiteral("line-gui-smoke.json"));
    QSaveFile reportFile(reportPath);
    if (!reportFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    reportFile.write(QJsonDocument(report).toJson(QJsonDocument::Indented));
    if (!reportFile.commit()) {
        return false;
    }
    return report.value(QStringLiteral("status")).toString() == QStringLiteral("PASS");
}


/**
 * Stores the global application settings to file or registry.
 */
void QC_ApplicationWindow::storeSettings() {
    RS_DEBUG->print("QC_ApplicationWindow::storeSettings()");

    if (RS_Settings::save_is_allowed)
    {
        RS_SETTINGS->beginGroup("/Geometry");
        RS_SETTINGS->writeEntry("/WindowWidth", width());
        RS_SETTINGS->writeEntry("/WindowHeight", height());
        RS_SETTINGS->writeEntry("/WindowX", x());
        RS_SETTINGS->writeEntry("/WindowY", y());
        QString geometry {saveGeometry().toBase64(QByteArray::Base64Encoding)};
        RS_SETTINGS->writeEntry("/WindowGeometry", geometry);
        RS_SETTINGS->writeEntry("/StateOfWidgets", QVariant (saveState()));
        RS_SETTINGS->writeEntry("/LeftDockArea", dock_areas.left->isChecked());
        RS_SETTINGS->writeEntry("/RightDockArea", dock_areas.right->isChecked());
        RS_SETTINGS->writeEntry("/TopDockArea", dock_areas.top->isChecked());
        RS_SETTINGS->writeEntry("/BottomDockArea", dock_areas.bottom->isChecked());
        RS_SETTINGS->writeEntry("/FloatingDockwidgets", dock_areas.floating->isChecked());
        RS_SETTINGS->endGroup();
        //save snapMode
        snapToolBar->saveSnapMode();
    }

    RS_DEBUG->print("QC_ApplicationWindow::storeSettings(): OK");
}

/**
 * Goes back to the previous menu or one step in the current action.
 */
void QC_ApplicationWindow::slotBack() {
    RS_GraphicView* graphicView = getGraphicView();
    if (graphicView) {
        graphicView->back();
    }
}

void QC_ApplicationWindow::slotKillAllActions() {
    RS_GraphicView* gv = getGraphicView();
    QC_MDIWindow* m = getMDIWindow();
    if (gv && m && m->getDocument()) {
        gv->killAllActions();

        RS_Selection s((RS_EntityContainer&)*m->getDocument(), gv);
        s.selectAll(false);
        RS_DIALOGFACTORY->updateSelectionWidget(
                    m->getDocument()->countSelected()
                    ,
                    m->getDocument()->totalSelectedLength()
                    );

        gv->redraw(RS2::RedrawAll);
    }
}


/**
 * Goes one step further in the current action.
 */
void QC_ApplicationWindow::slotEnter()
{
    RS_DEBUG->print("QC_ApplicationWindow::slotEnter(): begin\n");
    RS_GraphicView* graphicView = getGraphicView();
    if (graphicView)
    {
        graphicView->enter();
    }
    RS_DEBUG->print("QC_ApplicationWindow::slotEnter(): end\n");
}

/**
 * Sets the keyboard focus on the command line.
 */
void QC_ApplicationWindow::slotFocusCommandLine() {
//    if (commandWidget->isVisible()) {
        commandWidget->show();
        commandWidget->setFocus();
//    }
}


/**
 * Shows the given error on the command line.
 */
void QC_ApplicationWindow::slotError(const QString& msg) {
        commandWidget->appendHistory(msg);
}

void QC_ApplicationWindow::refreshKuubikProperties(int selectedCount, double totalLength)
{
    Q_UNUSED(selectedCount)
    Q_UNUSED(totalLength)
    if (kuubikPropertiesPalette == nullptr) return;
    QC_MDIWindow* mdi = getMDIWindow();
    RS_Document* document = mdi == nullptr ? nullptr : mdi->getDocument();
    if (kuubikPropertiesPalette->document() != document) {
        kuubikPropertiesPalette->setDocument(document);
    }
    if (document == nullptr) return;
    kuubikPropertiesPalette->refreshSelection(document->countSelected(),
                                               document->totalSelectedLength());
}

/**
 * Hands focus back to the application window. In the rare event
 * of a escape press from the layer widget (e.g after switching desktops
 * in XP).
 */
void QC_ApplicationWindow::slotFocus() {
    setFocus();
}

void QC_ApplicationWindow::slotWindowActivated(int index){
    if(index < 0 || index >= mdiAreaCAD->subWindowList().size()) return;
    slotWindowActivated(mdiAreaCAD->subWindowList().at(index));
}

/**
 * Called when a document window was activated.
 */
void QC_ApplicationWindow::slotWindowActivated(QMdiSubWindow* w, bool forced)
{
    RS_DEBUG->print("QC_ApplicationWindow::slotWindowActivated begin");

    if(w==nullptr) {
        if (kuubikCurrentLayerSelector != nullptr) {
            kuubikCurrentLayerSelector->setLayerList(nullptr);
        }
        if (kuubikPropertiesPalette != nullptr) {
            kuubikPropertiesPalette->setDocument(nullptr);
        }
        emit windowsChanged(false);
        activedMdiSubWindow=w;
        return;
    }

    QC_MDIWindow* m = qobject_cast<QC_MDIWindow*>(w);
    if(w==activedMdiSubWindow) {
        // Issue #2332 : still need to update File menu entries for the file name
        if (m != nullptr && m->getDocument() != nullptr) {
            enableFileActions(m);
            if (kuubikCurrentLayerSelector != nullptr) {
                kuubikCurrentLayerSelector->setLayerList(m->getDocument()->getLayerList());
            }
            if (kuubikPropertiesPalette != nullptr) {
                kuubikPropertiesPalette->setDocument(m->getDocument());
                kuubikPropertiesPalette->refreshSelection(
                    m->getDocument()->countSelected(),
                    m->getDocument()->totalSelectedLength());
            }
        }
        return;
    }
    activedMdiSubWindow=w;

    enableFileActions(m);

    if (m && m->getDocument()) {

        RS_DEBUG->print("QC_ApplicationWindow::slotWindowActivated: "
                        "document: %lu", m->getDocument()->getId());

        bool showByBlock = m->getDocument()->rtti()==RS2::EntityBlock;

        RS_LayerList *layerList = m->getDocument()->getLayerList();

        if (kuubikCurrentLayerSelector != nullptr) {
            kuubikCurrentLayerSelector->setLayerList(layerList);
        }
        if (kuubikPropertiesPalette != nullptr) {
            kuubikPropertiesPalette->setDocument(m->getDocument());
            kuubikPropertiesPalette->refreshSelection(
                m->getDocument()->countSelected(),
                m->getDocument()->totalSelectedLength());
        }

        layerWidget->setLayerList(layerList,showByBlock);

        if (layerTreeWidget != nullptr) {
            layerTreeWidget->setLayerList(layerList);
            layerTreeWidget->set_view(m->getGraphicView());
            layerTreeWidget->set_document(m->getDocument());
        }

        if (penPaletteWidget != nullptr){
            penPaletteWidget->setLayerList(layerList);
        }


        coordinateWidget->setGraphic(m->getGraphic());

        blockWidget->setBlockList(m->getDocument()->getBlockList());

        // Update all inserts in this graphic (blocks might have changed):
        m->getDocument()->updateInserts();
        // whether to enable undo/redo buttons
        m->getDocument()->setGUIButtons();
        m->getGraphicView()->redraw();

        // set snapmode from snap toolbar
        //actionHandler->updateSnapMode();

        // set pen from pen toolbar
        slotPenChanged(penToolBar->getPen());

        pen_wiz->setMdiWindow(m);
        pen_wiz->setVisible(usePenWizard());
        if (penPaletteWidget != nullptr){
         penPaletteWidget->setMdiWindow(m);
        }

        if (!forced)
        {
            // update toggle button status:
            if (m->getGraphic()) emit gridChanged(m->getGraphic()->isGridOn());
        }

        QG_GraphicView* view = m->getGraphicView();
        if (view)
        {
            actionHandler->set_view(view);
            actionHandler->set_document(m->getDocument());
            emit printPreviewChanged(view->isPrintPreview());
        }

        if(snapToolBar){
            actionHandler->slotSetSnaps(snapToolBar->getSnaps());
        }else {
            RS_DEBUG->print(RS_Debug::D_ERROR,"snapToolBar is nullptr\n");
        }
    }

    // show action options for active window only
    foreach (QMdiSubWindow* sw, mdiAreaCAD->subWindowList()) {
        QC_MDIWindow* sm = qobject_cast<QC_MDIWindow*>(sw);
        RS_ActionInterface* ai = sm->getGraphicView()->getCurrentAction();
        if (ai) {
            ai->hideOptions();
        }
    }
    if (m && m->getGraphicView()->getCurrentAction()) {
        m->getGraphicView()->getCurrentAction()->showOptions();
    }

    // Disable/Enable menu and toolbar items
    emit windowsChanged(m && m->getDocument());

    RS_DEBUG->print("RVT_PORT emit windowsChanged(true);");

    RS_DEBUG->print("QC_ApplicationWindow::slotWindowActivated end");
}

/**
 * Called when the menu 'windows' is about to be shown.
 * This is used to update the window list in the menu.
 */
void QC_ApplicationWindow::slotWindowsMenuAboutToShow() {

    RS_DEBUG->print( RS_Debug::D_NOTICE, "QC_ApplicationWindow::slotWindowsMenuAboutToShow");
	RS_SETTINGS->beginGroup("/WindowOptions");

	QMenu* menu;
	QAction* menuItem;
	bool tabbed = mdiAreaCAD->viewMode() == QMdiArea::TabbedView;
    windowsMenu->clear(); // this is a temporary menu; constructed on-demand

	menuItem = windowsMenu->addAction(tr("Ta&b mode"), this, SLOT(slotToggleTab()));
	menuItem->setCheckable(true);
	menuItem->setChecked(tabbed);

	menuItem = windowsMenu->addAction( tr("&Window mode"), this, SLOT(slotToggleTab()));
	menuItem->setCheckable(true);
	menuItem->setChecked(!tabbed);

	
	if (mdiAreaCAD->viewMode() == QMdiArea::TabbedView) {
		menu = new QMenu(tr("&Layout"), windowsMenu);
		windowsMenu->addMenu(menu);

		menuItem = menu->addAction(tr("Rounded"), this, SLOT(slotTabShapeRounded()));
		menuItem->setCheckable(true);
		menuItem->setChecked(RS_SETTINGS->readNumEntry("/TabShape") == RS2::Rounded);

		menuItem = menu->addAction(tr("Triangular"), this, SLOT(slotTabShapeTriangular()));
		menuItem->setCheckable(true);
		menuItem->setChecked(RS_SETTINGS->readNumEntry("/TabShape") == RS2::Triangular);

		menu->addSeparator();

		menuItem = menu->addAction(tr("North"), this, SLOT(slotTabPositionNorth()));
		menuItem->setCheckable(true);
		menuItem->setChecked(RS_SETTINGS->readNumEntry("/TabPosition") == RS2::North);

		menuItem = menu->addAction(tr("South"), this, SLOT(slotTabPositionSouth()));
		menuItem->setCheckable(true);
		menuItem->setChecked(RS_SETTINGS->readNumEntry("/TabPosition") == RS2::South);

		menuItem = menu->addAction(tr("East"), this, SLOT(slotTabPositionEast()));
		menuItem->setCheckable(true);
		menuItem->setChecked(RS_SETTINGS->readNumEntry("/TabPosition") == RS2::East);

		menuItem = menu->addAction(tr("West"), this, SLOT(slotTabPositionWest()));
		menuItem->setCheckable(true);
		menuItem->setChecked(RS_SETTINGS->readNumEntry("/TabPosition") == RS2::West);

	} else {
		menu = new QMenu(tr("&Arrange"), windowsMenu);
		windowsMenu->addMenu(menu);

		menuItem = menu->addAction(tr("&Maximized"), this, SLOT(slotSetMaximized()));
		menuItem->setCheckable(true);
		menuItem->setChecked(RS_SETTINGS->readNumEntry("/SubWindowMode") == RS2::Maximized);

        menu->addAction(tr("&Cascade"), this, SLOT(slotCascade()));
        menu->addAction(tr("&Tile"), this, SLOT(slotTile()));
        menu->addAction(tr("Tile &Vertically"), this, SLOT(slotTileVertical()));
        menu->addAction(tr("Tile &Horizontally"), this, SLOT(slotTileHorizontal()));
	}
	

	RS_SETTINGS->endGroup();
        
    windowsMenu->addSeparator();
    QMdiSubWindow* active= mdiAreaCAD->activeSubWindow();
    for (int i=0; i< window_list.size(); ++i) {
        QString title = window_list.at(i)->windowTitle();
        if (title.contains("[*]")) { // modification mark placeholder
            int idx = title.lastIndexOf("[*]");
            if (window_list.at(i)->isWindowModified()) {
                title.replace(idx, 3, "*");
            } else {
                title.remove(idx, 3);
            }
        }
        QAction *id = windowsMenu->addAction(title,
                                         this, SLOT(slotWindowsMenuActivated(bool)));
        id->setCheckable(true);
        id->setData(i);
        id->setChecked(window_list.at(i)==active);
    }
}

/**
 * Called when the user selects a document window from the
 * window list.
 */
void QC_ApplicationWindow::slotWindowsMenuActivated(bool /*id*/) {
    RS_DEBUG->print("QC_ApplicationWindow::slotWindowsMenuActivated");

    int ii = qobject_cast<QAction*>(sender())->data().toInt();
    QMdiSubWindow* w = mdiAreaCAD->subWindowList().at(ii);
    if (w) {
        if(w==mdiAreaCAD->activeSubWindow()) {
            return;
        }

		doActivate(w);
    }
}

/**
 * Cascade MDI windows
 */
void QC_ApplicationWindow::slotTile() {
	doArrangeWindows(RS2::Tile, true);
        mdiAreaCAD->tileSubWindows();
        slotZoomAuto();
}
//auto zoom the graphicView of sub-windows
void QC_ApplicationWindow::slotZoomAuto() {
    QList<QMdiSubWindow *> windows = mdiAreaCAD->subWindowList();
    for(int i=0;i<windows.size();i++){
        QMdiSubWindow *window = windows.at(i);
        qobject_cast<QC_MDIWindow*>(window)->slotZoomAuto();
    }
}

/**
 * Cascade MDI windows
 */
void QC_ApplicationWindow::slotCascade() {
//    mdiAreaCAD->cascadeSubWindows();
//return;
	doArrangeWindows(RS2::Cascade, true);
	QList<QMdiSubWindow *> windows = mdiAreaCAD->subWindowList();
    switch(windows.size()){
    case 1:
        //mdiAreaCAD->tileSubWindows();
		slotTile();
    case 0:
        return;
    default: {
        QMdiSubWindow* active=mdiAreaCAD->activeSubWindow();
        for (int i=0; i<windows.size(); ++i) {
            windows.at(i)->showNormal();
        }
        mdiAreaCAD->cascadeSubWindows();
        //find displacement by linear-regression
        double mi=0.,mi2=0.,mw=0.,miw=0.,mh=0.,mih=0.;
        for (int i=0; i<windows.size(); ++i) {
                mi += i;
                mi2 += i*i;
                double w=windows.at(i)->pos().x();
                mw += w;
                miw += i*w;
                double h=windows.at(i)->pos().y();
                mh += h;
                mih += i*h;
        }
        mi2 *= windows.size();
        miw *= windows.size();
        mih *= windows.size();
        double d=1./(mi2 - mi*mi);
        double disX=(miw-mi*mw)*d;
        double disY=(mih-mi*mh)*d;
        //End of Linear Regression
        //
        QMdiSubWindow *window = windows.first();
        QRect geo=window->geometry();
        QRect frame=window->frameGeometry();
//        std::cout<<"Frame=:"<<( frame.height() - geo.height())<<std::endl;
        int width= mdiAreaCAD->width() -( frame.width() - geo.width())- disX*(windows.size()-1);
        int height= mdiAreaCAD->height() -( frame.width() - geo.width())- disY*(windows.size()-1);
        if(width<=0 || height<=0) {
            return;
        }
        for (int i=0; i<windows.size(); ++i) {
            window = windows.at(i);
//            std::cout<<"window:("<<i<<"): pos()="<<(window->pos().x())<<" "<<(window->pos().y())<<std::endl;
            geo=window->geometry();
//            if(i==active) {
//                    window->setWindowState(Qt::WindowActive);
//            }else{
//                    window->setWindowState(Qt::WindowNoState);
//            }
            window->setGeometry(geo.x(),geo.y(),width,height);
            qobject_cast<QC_MDIWindow*>(window)->slotZoomAuto();
        }
        mdiAreaCAD->setActiveSubWindow(active);
//        windows.at(active)->activateWindow();
//        windows.at(active)->raise();
//        windows.at(active)->setFocus();
    }
    }
}


/**
 * Tiles MDI windows horizontally.
 */
void QC_ApplicationWindow::slotTileHorizontal() {

    RS_DEBUG->print("QC_ApplicationWindow::slotTileHorizontal");
	doArrangeWindows(RS2::TileHorizontal, true);
	
    // primitive horizontal tiling
    QList<QMdiSubWindow *> windows = mdiAreaCAD->subWindowList();
    if (windows.count()<=1) {
		slotTile();
        return;
    }
    for (int i=0; i<windows.count(); ++i) {
        QMdiSubWindow *window = windows.at(i);
        window->lower();
        window->showNormal();
    }
    int heightForEach = mdiAreaCAD->height() / windows.count();
    int y = 0;
    for (int i=0; i<windows.count(); ++i) {
        QMdiSubWindow *window = windows.at(i);
        int preferredHeight = window->minimumHeight()
                              + window->parentWidget()->baseSize().height();
        int actHeight = qMax(heightForEach, preferredHeight);

        window->setGeometry(0, y, mdiAreaCAD->width(), actHeight);
         qobject_cast<QC_MDIWindow*>(window)->slotZoomAuto();
        y+=actHeight;
    }
    mdiAreaCAD->activeSubWindow()->raise();
}


/**
 * Tiles MDI windows vertically.
 */
void QC_ApplicationWindow::slotTileVertical() {
	
    RS_DEBUG->print("QC_ApplicationWindow::slotTileVertical()");
	doArrangeWindows(RS2::TileVertical, true);
	
    // primitive horizontal tiling
    QList<QMdiSubWindow *> windows = mdiAreaCAD->subWindowList();
    if (windows.count()<=1) {
		slotTile();
        return;
    }
    for (int i=0; i<windows.count(); ++i) {
        QMdiSubWindow *window = windows.at(i);
        window->lower();
        window->showNormal();
    }
    int widthForEach = mdiAreaCAD->width() / windows.count();
    int x = 0;
    for (int i=0; i<windows.count(); ++i) {
        QMdiSubWindow *window = windows.at(i);
        int preferredWidth = window->minimumWidth()
                              + window->parentWidget()->baseSize().width();
        int actWidth = qMax(widthForEach, preferredWidth);

        window->setGeometry(x, 0, actWidth, mdiAreaCAD->height());
         qobject_cast<QC_MDIWindow*>(window)->slotZoomAuto();
        x+=actWidth;
    }
    mdiAreaCAD->activeSubWindow()->raise();
}

void QC_ApplicationWindow::slotSetMaximized()
{
	doArrangeWindows(RS2::Maximized);
}

void QC_ApplicationWindow::slotTabShapeRounded()
{
	setTabLayout(RS2::Rounded, RS2::AnyPosition);
}

void QC_ApplicationWindow::slotTabShapeTriangular()
{
	setTabLayout(RS2::Triangular, RS2::AnyPosition);
}

void QC_ApplicationWindow::slotTabPositionNorth()
{
	setTabLayout(RS2::AnyShape, RS2::North);
}

void QC_ApplicationWindow::slotTabPositionSouth()
{
	setTabLayout(RS2::AnyShape, RS2::South);
}

void QC_ApplicationWindow::slotTabPositionEast()
{
	setTabLayout(RS2::AnyShape, RS2::East);
}

void QC_ApplicationWindow::slotTabPositionWest()
{
	setTabLayout(RS2::AnyShape, RS2::West);
}

/**
 * toggles between subwindow and tab mode for the MdiArea
 */
void QC_ApplicationWindow::slotToggleTab()
{
    if (mdiAreaCAD->viewMode() == QMdiArea::SubWindowView)
    {
		RS_SETTINGS->beginGroup("Startup");
		RS_SETTINGS->writeEntry("/TabMode", 1);
		RS_SETTINGS->endGroup();
        mdiAreaCAD->setViewMode(QMdiArea::TabbedView);
		QList<QTabBar *> tabBarList = mdiAreaCAD->findChildren<QTabBar*>();
		QTabBar *tabBar = tabBarList.at(0);
		if (tabBar) {
			tabBar->setExpanding(false);
		}
        QList<QMdiSubWindow *> windows = mdiAreaCAD->subWindowList();
        QMdiSubWindow* active=mdiAreaCAD->activeSubWindow();
        for(int i=0;i<windows.size();i++){
            QMdiSubWindow* m=windows.at(i);
            m->hide();
            if(m!=active){
                m->lower();
            }else{
                m->raise();
            }
			slotSetMaximized();
            qobject_cast<QC_MDIWindow*>(m)->slotZoomAuto();
        }
    }
    else
    {
		RS_SETTINGS->beginGroup("Startup");
		RS_SETTINGS->writeEntry("/TabMode", 0);
		RS_SETTINGS->endGroup();
        mdiAreaCAD->setViewMode(QMdiArea::SubWindowView);
		doArrangeWindows(RS2::CurrentMode);
    }
}

/**
 * Called when something changed in the pen tool bar
 * (e.g. color, width, style).
 */
void QC_ApplicationWindow::slotPenChanged(RS_Pen pen) {
    RS_DEBUG->print("QC_ApplicationWindow::slotPenChanged() begin");

    RS_DEBUG->print("Setting active pen...");

    QC_MDIWindow* m = getMDIWindow();
    if (m) {
        m->slotPenChanged(pen);
    }

    RS_DEBUG->print("QC_ApplicationWindow::slotPenChanged() end");
}

///**
// * Called when something changed in the snaps tool bar
// */
//void QC_ApplicationWindow::slotSnapsChanged(const RS_SnapMode& snaps) {
//    RS_DEBUG->print("QC_ApplicationWindow::slotSnapsChanged() begin");

//    actionHandler->slotSetSnaps(snaps);
//}



/**
 * Creates a new MDI window with the given document or a new
 *  document if 'doc' is nullptr.
 */

QC_MDIWindow* QC_ApplicationWindow::slotFileNew(RS_Document* doc) {

    RS_DEBUG->print("QC_ApplicationWindow::slotFileNew() begin");

    QSettings settings;
    static int id = 0;
    id++;

    statusBar()->showMessage(tr("Creating new file..."));

    RS_DEBUG->print("  creating MDI window");

    QC_MDIWindow *w = new QC_MDIWindow(doc, mdiAreaCAD, {});

    window_list << w;

    RS_SETTINGS->beginGroup("/Appearance");
    int aa = RS_SETTINGS->readNumEntry("/Antialiasing", 0);
    int scrollbars = RS_SETTINGS->readNumEntry("/ScrollBars", 1);
    int cursor_hiding = RS_SETTINGS->readNumEntry("/cursor_hiding", 0);
    RS_SETTINGS->endGroup();

    QG_GraphicView* view = w->getGraphicView();

    view->setAntialiasing(aa);
    view->setCursorHiding(cursor_hiding);
    view->device = settings.value("Hardware/Device", "Mouse").toString();
    if (scrollbars) view->addScrollbars();

    settings.beginGroup("Activators");
    auto activators = settings.childKeys();
    settings.endGroup();

    foreach (auto activator, activators)
    {
        auto menu_name = settings.value("Activators/" + activator).toString();
        auto path = QString("CustomMenus/%1").arg(menu_name);
        auto a_list = settings.value(path).toStringList();
        auto menu = new QMenu(activator, view);
        menu->setObjectName(menu_name);
        foreach (auto key, a_list)
        {
            menu->addAction(a_map[key]);
        }
        view->setMenu(activator, menu);
    }

    connect(view, SIGNAL(gridStatusChanged(QString)),
            this, SLOT(updateGridStatus(QString)));

    actionHandler->set_view(view);
    actionHandler->set_document(w->getDocument());

    if (w->getDocument()->rtti()==RS2::EntityBlock) {
        w->setWindowTitle(tr("Block '%1'").arg(((RS_Block*)(w->getDocument()))->getName()) + "[*]");
    } else {
        w->setWindowTitle(tr("unnamed document %1").arg(id) + "[*]");
    }

    //check for draft mode

    if (settings.value("Appearance/DraftMode", 0).toBool())
    {
        QString draft_string = " ["+tr("Draft Mode")+"]";
        w->getGraphicView()->setDraftMode(true);
        QString title = w->windowTitle();
        w->setWindowTitle(title + draft_string);
    }

    w->setWindowIcon(QIcon(":/main/document.png"));

    RS_DEBUG->print("  adding listeners");
    RS_Graphic* graphic = w->getDocument()->getGraphic();

    RS_LayerList* layerList = w->getDocument()->getLayerList();

    if (layerWidget != nullptr) {
        layerWidget->setLayerList(layerList, false);
    }

    if (penPaletteWidget != nullptr){
        penPaletteWidget->setLayerList(layerList);
    }

    if(layerTreeWidget != nullptr) {
        layerTreeWidget->setLayerList(layerList);
        layerTreeWidget->set_view(view);
        layerTreeWidget->set_document(w->getDocument());
    }

    if(blockWidget) {
        blockWidget->setBlockList(w->getDocument()->getBlockList());
    }
    if (graphic) {
        // Link the graphic's layer list to the pen tool bar
        graphic->addLayerListListener(penToolBar);
        // Link the layer list to the layer widget
        graphic->addLayerListListener(layerWidget);

        if (layerTreeWidget != nullptr) {
            graphic->addLayerListListener(layerTreeWidget);
        }

        // Link the block list to the block widget
        graphic->addBlockListListener(blockWidget);
    }
	// Link the dialog factory to the coordinate widget:
	if( coordinateWidget){
		coordinateWidget->setGraphic(graphic );
	}
	// Link the dialog factory to the mouse widget:
	QG_DIALOGFACTORY->setMouseWidget(mouseWidget);
	QG_DIALOGFACTORY->setCoordinateWidget(coordinateWidget);
	QG_DIALOGFACTORY->setSelectionWidget(selectionWidget);
	// Link the dialog factory to the option widget:
	//QG_DIALOGFACTORY->setOptionWidget(optionWidget);
	// Link the dialog factory to the command widget:
	QG_DIALOGFACTORY->setCommandWidget(commandWidget);

    mdiAreaCAD->addSubWindow(w);

    RS_DEBUG->print("  showing MDI window");
	doActivate(w);
	doArrangeWindows(RS2::CurrentMode);
    statusBar()->showMessage(tr("New Drawing created."), 2000);

    layerWidget->activateLayer(0);

    RS_DEBUG->print("QC_ApplicationWindow::slotFileNew() OK");

    return w;
}

/**
 * Helper function for Menu file -> New & New....
 */
bool QC_ApplicationWindow::slotFileNewHelper(QString fileName, QC_MDIWindow* w) {
    RS_DEBUG->print("QC_ApplicationWindow::slotFileNewHelper()");
    bool ret = false;
    RS2::FormatType type = RS2::FormatDXFRW;

    QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

    RS_DEBUG->print("QC_ApplicationWindow::slotFileNewHelper: creating new doc window");
    /*QC_MDIWindow* */ w = slotFileNew();
    qApp->processEvents(QEventLoop::AllEvents, 1000);

    // link the layer widget to the new document:
    RS_LayerList *layerList = w->getDocument()->getLayerList();
    layerWidget->setLayerList(layerList, false);
    if (layerTreeWidget != nullptr)
        layerTreeWidget->setLayerList(layerList);

    if (penPaletteWidget != nullptr){
        penPaletteWidget->  setLayerList(layerList);
    }


    // link the block widget to the new document:
    blockWidget->setBlockList(w->getDocument()->getBlockList());
    // link coordinate widget to graphic
    coordinateWidget->setGraphic(w->getGraphic());

    qApp->processEvents(QEventLoop::AllEvents, 1000);

    // loads the template file in the new view:
    if (!fileName.isEmpty()) {
        ret = w->slotFileNewTemplate(fileName, type);
    } else
        //new without template is OK;
        ret = true;

    if (!ret) {
        // error loading template
        QApplication::restoreOverrideCursor();
        return ret;
    }

    RS_DEBUG->print("QC_ApplicationWindow::slotFileNewHelper: load Template: OK");

    layerWidget->slotUpdateLayerList();
    if (layerTreeWidget != nullptr)
        layerTreeWidget->slotFilteringMaskChanged();

    RS_DEBUG->print("QC_ApplicationWindow::slotFileNewHelper: update coordinate widget");
    // update coordinate widget format:
    RS_DIALOGFACTORY->updateCoordinateWidget(RS_Vector(0.0,0.0),
                                             RS_Vector(0.0,0.0), true);

    if (!fileName.isEmpty()) {
        QString message=tr("New document from template: ")+fileName;
        commandWidget->appendHistory(message);
        statusBar()->showMessage(message, 2000);
    }
    if (w->getGraphic()) {
        emit(gridChanged(w->getGraphic()->isGridOn()));
    }

    QApplication::restoreOverrideCursor();
    RS_DEBUG->print("QC_ApplicationWindow::slotFileNewHelper() OK");
    return ret;
}

/**
 * Menu file -> New (using a predefined Template).
 */
void QC_ApplicationWindow::slotFileNewNew() {
    RS_DEBUG->print("QC_ApplicationWindow::slotFileNewNew()");

//    RS2::FormatType type = RS2::FormatDXFRW;
    //tried to load template file indicated in RS_Settings
    RS_SETTINGS->beginGroup("/Paths");
    QString fileName = RS_SETTINGS->readEntry("/Template");
    RS_SETTINGS->endGroup();
/*    QFileInfo finfo(fileName);
    if (!fileName.isEmpty() && finfo.isReadable()) {
        slotFileNewTemplate(fileName, RS2::FormatDXFRW);
        return;
    }*/

	if (!slotFileNewHelper(fileName)) {
        // error opening template
        RS_DEBUG->print("QC_ApplicationWindow::slotFileNewNew: load Template failed");
    } else
        RS_DEBUG->print("QC_ApplicationWindow::slotFileNewNew() OK");
}

/**
 * Menu file -> New with Template.
 */
void QC_ApplicationWindow::slotFileNewTemplate() {
    RS_DEBUG->print("QC_ApplicationWindow::slotFileNewTemplate()");

    RS2::FormatType type = RS2::FormatDXFRW;
    QG_FileDialog dlg(this);
    QString fileName = dlg.getOpenFile(&type);

    if (fileName.isEmpty()) {
           statusBar()->showMessage(tr("Select Template aborted"), 2000);
           return;
       }

    RS_DEBUG->print("QC_ApplicationWindow::slotFileNewTemplate: creating new doc window");
    // Create new document window:
    QMdiSubWindow* old=activedMdiSubWindow;
    QRect geo;
    bool maximized=false;
    if(old != nullptr ) {//save old geometry
        geo=old->geometry();
        maximized=old->isMaximized();
    }
    QC_MDIWindow* w =nullptr;
	if (!slotFileNewHelper(fileName, w)) {
        // error
        QString msg=tr("Cannot open the file\n%1\nPlease "
                       "check the permissions.").arg(fileName);
        commandWidget->appendHistory(msg);
        QMessageBox::information(this, QMessageBox::tr("Warning"),
                                 msg,QMessageBox::Ok);
        //file opening failed, clean up QC_MDIWindow and QMdiSubWindow
        if (w) {
            slotFilePrintPreview(false);
            doClose(w); //force closing, without asking user for confirmation
        }
        QMdiSubWindow* active=mdiAreaCAD->currentSubWindow();
        activedMdiSubWindow=nullptr; //to allow reactivate the previous active
        if( active){//restore old geometry
            mdiAreaCAD->setActiveSubWindow(active);
            active->raise();
            active->setFocus();
            if(old==nullptr || maximized){
                active->showMaximized();
            }else{
                active->setGeometry(geo);
            }
        }
        RS_DEBUG->print("QC_ApplicationWindow::slotFileNewTemplate: load Template failed");
    } else
        RS_DEBUG->print("QC_ApplicationWindow::slotFileNewTemplate() OK");
}


/**
 * Menu file -> open.
 */
void QC_ApplicationWindow::slotFileOpen() {
    RS_DEBUG->print("QC_ApplicationWindow::slotFileOpen()");

    RS_DEBUG->print("QC_ApplicationWindow::slotFileOpen() 001");
    RS2::FormatType type = RS2::FormatUnknown;
    RS_DEBUG->print("QC_ApplicationWindow::slotFileOpen() 002");
    QG_FileDialog dlg(this);
    QString fileName = dlg.getOpenFile(&type);
    RS_DEBUG->print("QC_ApplicationWindow::slotFileOpen() 003");
    slotFileOpen(fileName, type);
    RS_DEBUG->print("QC_ApplicationWindow::slotFileOpen(): OK");
}


/**
 *
 *	\brief	- Format a string that hold a file name path
 *						  such a way that it can displayed on the
 *						  windows title bar.
 *
 *	\author		Claude Sylvain
 *	\date			30 July 2011
 *	Last modified:
 *
 *	Parameters:		const QString &qstring_in:
 *							String to format (in).
 *
 *						QString &qstring_out:
 *							Formatted string (out).
 *
 *	Returns:			void
 *	*/

QString QC_ApplicationWindow::
    format_filename_caption(const QString &qstring_in)
{
	QFileInfo info = QFileInfo(qstring_in);
	return info.fileName(); // don't include the full path
}

/*	*
 *	Function name:
 *	Description:
 *	Author(s):		..., Claude Sylvain
 *	Created:			?
 *	Last modified:	30 July 2011
 *
 *	Parameters:		const QString& fileName:
 *							...
 *
 *						RS2::FormatType type:
 *							...
 *
 *	Returns:			void
 *	Notes:			Menu file -> open.
 *	*/
void QC_ApplicationWindow::
        slotFileOpen(const QString& fileName, RS2::FormatType type)
{
    RS_DEBUG->print("QC_ApplicationWindow::slotFileOpen(..)");

    QSettings settings;

    QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

    if ( QFileInfo(fileName).exists())
         {
        RS_DEBUG->print("QC_ApplicationWindow::slotFileOpen: creating new doc window");
        if (openedFiles.indexOf(fileName) >=0) {
            QString message=tr("Warning: File already opened : ")+fileName;
            commandWidget->appendHistory(message);
            statusBar()->showMessage(message, 2000);
        }
        // Create new document window:
		QMdiSubWindow* old=activedMdiSubWindow;
        QRect geo;
        //bool maximized=false;

        QC_MDIWindow* w = slotFileNew(nullptr);
        // RVT_PORT qApp->processEvents(1000);
        qApp->processEvents(QEventLoop::AllEvents, 1000);

        RS_DEBUG->print("QC_ApplicationWindow::slotFileOpen: linking layer list");
             RS_LayerList *layerList = w->getDocument()->getLayerList();
        // link the layer widget to the new document:

        layerWidget->setLayerList(layerList, false);
        if (layerTreeWidget)
            layerTreeWidget->setLayerList(layerList);
        if (penPaletteWidget != nullptr){
            penPaletteWidget->setLayerList(layerList);
         }
        // link the block widget to the new document:
        blockWidget->setBlockList(w->getDocument()->getBlockList());
        // link coordinate widget to graphic
        coordinateWidget->setGraphic(w->getGraphic());

        RS_DEBUG->print("QC_ApplicationWindow::slotFileOpen: open file");

        qApp->processEvents(QEventLoop::AllEvents, 1000);

        if(old != nullptr) {//save old geometry
            geo=old->geometry();
            //maximized=old->isMaximized();
        }

        // open the file in the new view:
        bool success=false;
        if (QFileInfo( fileName).exists()) {
            success = w->slotFileOpen( fileName, type);
        }
        else {
            QString msg=tr("Cannot open the file\n%1\nPlease "
                           "check its existence and permissions.")
                    .arg( fileName);
            commandWidget->appendHistory( msg);
            QMessageBox::information( this, QMessageBox::tr("Warning"), msg, QMessageBox::Ok);
        }
        if (!success) {
               // error
               QApplication::restoreOverrideCursor();

               //file opening failed, clean up QC_MDIWindow and QMdiSubWindow
               slotFilePrintPreview(false);
               doClose(w); //force closing, without asking user for confirmation
               return;
        }

		slotWindowActivated(w);

        RS_DEBUG->print("QC_ApplicationWindow::slotFileOpen: open file: OK");

        RS_DEBUG->print("QC_ApplicationWindow::slotFileOpen: update recent file menu: 1");

        // update recent files menu:
        recentFiles->add(fileName);
        openedFiles.push_back(fileName);
        layerWidget->slotUpdateLayerList();
        if (layerTreeWidget != nullptr)
            layerTreeWidget->slotFilteringMaskChanged();

        auto graphic = w->getGraphic();
        if (graphic)
        {
            if (int objects_removed = graphic->clean())
            {
                auto msg = QObject::tr("Invalid objects removed:");
                commandWidget->appendHistory(msg + " " + QString::number(objects_removed));
            }
            emit(gridChanged(graphic->isGridOn()));
        }

        RS_DEBUG->print("QC_ApplicationWindow::slotFileOpen: set caption");


                /*	Format and set caption.
                 *	----------------------- */
        w->setWindowTitle(format_filename_caption(fileName) + "[*]");

		if (mdiAreaCAD->viewMode() == QMdiArea::TabbedView) {
			QList<QTabBar *> tabBarList = mdiAreaCAD->findChildren<QTabBar*>();
			QTabBar *tabBar = tabBarList.at(0);
			if (tabBar) {
				tabBar->setExpanding(false);
				tabBar->setTabToolTip(tabBar->currentIndex(), fileName);
			}
		}
		else
			doArrangeWindows(RS2::CurrentMode);

		RS_SETTINGS->beginGroup("/CADPreferences");
		if (RS_SETTINGS->readNumEntry("/AutoZoomDrawing"))
			w->getGraphicView()->zoomAuto(false);
		RS_SETTINGS->endGroup();

        if (settings.value("Appearance/DraftMode", 0).toBool())
        {
            QString draft_string = " ["+tr("Draft Mode")+"]";
            w->getGraphicView()->setDraftMode(true);
            w->getGraphicView()->redraw();
            QString title = w->windowTitle();
            w->setWindowTitle(title + draft_string);
        }

        RS_DEBUG->print("QC_ApplicationWindow::slotFileOpen: set caption: OK");

        RS_DEBUG->print("QC_ApplicationWindow::slotFileOpen: update coordinate widget");
        // update coordinate widget format:
        RS_DIALOGFACTORY->updateCoordinateWidget(RS_Vector(0.0,0.0),
                RS_Vector(0.0,0.0),
                true);
        RS_DEBUG->print("QC_ApplicationWindow::slotFileOpen: update coordinate widget: OK");

        QString message=tr("Loaded document: ")+fileName;
        commandWidget->appendHistory(message);
        statusBar()->showMessage(message, 2000);

	} else {
		QG_DIALOGFACTORY->commandMessage(tr("File '%1' does not exist. Opening aborted").arg(fileName));
        statusBar()->showMessage(tr("Opening aborted"), 2000);
    }

    QApplication::restoreOverrideCursor();
    RS_DEBUG->print("QC_ApplicationWindow::slotFileOpen(..) OK");
}

void QC_ApplicationWindow::slotFileOpen(const QString& fileName) {
    slotFileOpen(fileName, RS2::FormatUnknown);
}


/**
 * Menu file -> save.
 */
void QC_ApplicationWindow::slotFileSave() {
    RS_DEBUG->print("QC_ApplicationWindow::slotFileSave()");

	if (doSave(getMDIWindow()))
		recentFiles->updateRecentFilesMenu();
}



/**
 * Menu file -> save as.
 */
void QC_ApplicationWindow::slotFileSaveAs() {
    RS_DEBUG->print("QC_ApplicationWindow::slotFileSaveAs()");
	if (doSave(getMDIWindow(), true))
		recentFiles->updateRecentFilesMenu();
}

bool QC_ApplicationWindow::slotFileSaveAll()
{
	QC_MDIWindow* current = getMDIWindow();
    bool result {true};
	for (auto w : window_list) {
		if (w && w->getDocument()->isModified()) {
			result = doSave(w);
			if (!result) {
				statusBar()->showMessage(tr("Save All cancelled"), 2000);
				break;
			}
		}
	}
	doActivate(current);
	recentFiles->updateRecentFilesMenu();
	return result;
}



/**
 * Autosave.
 */
void QC_ApplicationWindow::slotFileAutoSave() {
    RS_DEBUG->print("QC_ApplicationWindow::slotFileAutoSave(): begin");

    auto groupGuard = RS_SETTINGS->beginGroupGuard("/Defaults");
    if (RS_SETTINGS->readNumEntry("/AutoBackupDocument", 1) == 0) {
        RS_DEBUG->print(RS_Debug::D_INFORMATIONAL, "QC_ApplicationWindow::%s: /Defaults/AutoBackupDocument is disabled\n", __func__);
        startAutoSave(false);
        return;
    }

    statusBar()->showMessage(tr("Auto-saving drawing..."), 2000);

    QC_MDIWindow* w = getMDIWindow();
    if (w) {
        bool cancelled;
        if (w->slotFileSave(cancelled, true)) {
            // auto-save cannot be cancelled by user, so the
            // "cancelled" parameter is a dummy
            statusBar()->showMessage(tr("Auto-saved drawing"), 2000);
        } else {
            // error
            m_autosaveTimer->stop();
            QMessageBox::information(this, QMessageBox::tr("Warning"),
                                     tr("Cannot auto-save the file\n%1\nPlease "
                                        "check the permissions.\n"
                                        "Auto-save disabled.")
                                     .arg(w->getDocument()->getAutoSaveFilename()),
                                     QMessageBox::Ok);
            statusBar()->showMessage(tr("Auto-saving failed"), 2000);
        }
    }
}



/**
 * Menu file -> export.
 */
void QC_ApplicationWindow::slotFileExport() {
    RS_DEBUG->print("QC_ApplicationWindow::slotFileExport()");

    statusBar()->showMessage(tr("Exporting drawing..."), 2000);

    QC_MDIWindow* w = getMDIWindow();
    QString fn;
    if (w) {

        // read default settings:
        RS_SETTINGS->beginGroup("/Export");
        QString defDir = RS_SETTINGS->readEntry("/ExportImage", RS_SYSTEM->getHomeDir());
		QString defFilter = RS_SETTINGS->readEntry("/ExportImageFilter",
													   QString("%1 (%2)(*.%2)").arg(QG_DialogFactory::extToFormat("png")).arg("png"));
        RS_SETTINGS->endGroup();

        bool cancel = false;

        QStringList filters;
        QList<QByteArray> supportedImageFormats = QImageWriter::supportedImageFormats();
        supportedImageFormats.push_back("svg"); // add svg

        for (QString format: supportedImageFormats) {
            format = format.toLower();
            QString st;
            if (format=="jpeg" || format=="tiff") {
                // Don't add the aliases
			} else {
				st = QString("%1 (%2)(*.%2)")
						.arg(QG_DialogFactory::extToFormat(format))
						.arg(format);
            }
            if (st.length()>0)
                filters.push_back(st);
        }
        // revise list of filters
        filters.removeDuplicates();
        filters.sort();

        // set dialog options: filters, mode, accept, directory, filename
        QFileDialog fileDlg(this, tr("Export as"));

        fileDlg.setNameFilters(filters);
        fileDlg.setFileMode(QFileDialog::AnyFile);
        fileDlg.selectNameFilter(defFilter);
        fileDlg.setAcceptMode(QFileDialog::AcceptSave);
        fileDlg.setDirectory(defDir);
        fn = QFileInfo(w->getDocument()->getFilename()).baseName();
        if(fn==nullptr)
            fn = "unnamed";
        fileDlg.selectFile(fn);

        if (fileDlg.exec()==QDialog::Accepted) {
            QStringList files = fileDlg.selectedFiles();
            if (!files.isEmpty())
                fn = files[0];
            cancel = false;
        } else {
            cancel = true;
        }

        // store new default settings:
        if (!cancel) {
            RS_SETTINGS->beginGroup("/Export");
            RS_SETTINGS->writeEntry("/ExportImage", QFileInfo(fn).absolutePath());
            RS_SETTINGS->writeEntry("/ExportImageFilter",
                                    fileDlg.selectedNameFilter());
            RS_SETTINGS->endGroup();

            // find out extension:

            QString filter = fileDlg.selectedNameFilter();
            QString format = "";
            int i = filter.indexOf("(*.");
            if (i!=-1) {
                int i2 = filter.indexOf(QRegExp("[) ]"), i);
                format = filter.mid(i+3, i2-(i+3));
                format = format.toUpper();
            }

            // append extension to file:
            if (!QFileInfo(fn).fileName().contains(".")) {
                fn.push_back("." + format.toLower());
            }

            // show options dialog:
            QG_ImageOptionsDialog dlg(this);
            w->getGraphic()->calculateBorders();
            dlg.setGraphicSize(w->getGraphic()->getSize()*2.);
            if (dlg.exec()) {

                //QSize size = dlg.getSize();
                //QSize borders = dlg.getBorders();
                //bool black = dlg.isBackgroundBlack();
                //bool bw = dlg.isBlackWhite();

                bool ret = slotFileExport(fn, format, dlg.getSize(), dlg.getBorders(),
                            dlg.isBackgroundBlack(), dlg.isBlackWhite());
                if (ret) {
                    QString message = tr("Exported: %1").arg(fn);
                    statusBar()->showMessage(message, 2000);
                    commandWidget->appendHistory(message);
                }
            }
        }
    }

}



/**
 * Exports the drawing as a bitmap or another picture format.
 *
 * @param name File name.
 * @param format File format (e.g. "png")
 * @param size Size of the bitmap in pixel
 * @param black true: Black background, false: white
 * @param bw true: black/white export, false: color
 */
bool QC_ApplicationWindow::slotFileExport(const QString& name,
        const QString& format, QSize size, QSize borders, bool black, bool bw) {

    QC_MDIWindow* w = getMDIWindow();
    if (w==nullptr) {
        RS_DEBUG->print(RS_Debug::D_WARNING,
                "QC_ApplicationWindow::slotFileExport: "
                "no window opened");
        return false;
    }

    RS_Graphic* graphic = w->getDocument()->getGraphic();
    if (graphic==nullptr) {
        RS_DEBUG->print(RS_Debug::D_WARNING,
                "QC_ApplicationWindow::slotFileExport: "
                "no graphic");
        return false;
    }

    statusBar()->showMessage(tr("Exporting..."));
    QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

    bool ret = false;
    // set vars for normal pictures and vectors (svg)
    QPixmap* picture = new QPixmap(size);

    QSvgGenerator* vector = new QSvgGenerator();

    // set buffer var
    QPaintDevice* buffer;

    if(format.toLower() != "svg") {
        buffer = picture;
    } else {
        vector->setSize(size);
        vector->setViewBox(QRectF(QPointF(0,0),size));
        vector->setFileName(name);
        buffer = vector;
    }

    // set painter with buffer
    RS_PainterQt painter(buffer);

    if (black) {
        painter.setBackground( Qt::black);
        if (bw) {
            painter.setDrawingMode( RS2::ModeWB);
        }
    }
    else {
        painter.setBackground(Qt::white);
        if (bw) {
            painter.setDrawingMode( RS2::ModeBW);
        }
    }

    painter.eraseRect(0,0, size.width(), size.height());

	RS_StaticGraphicView gv(size.width(), size.height(), &painter, &borders);
    if (black) {
		gv.setBackground(Qt::black);
    } else {
		gv.setBackground(Qt::white);
    }
    gv.setContainer(graphic);
    gv.zoomAuto(false);
	gv.drawEntity(&painter, gv.getContainer());

    // end the picture output
    if(format.toLower() != "svg")
    {
        // RVT_PORT QImageIO iio;
        QImageWriter iio;
        QImage img = picture->toImage();
        // RVT_PORT iio.setImage(img);
        iio.setFileName(name);
        iio.setFormat(format.toLatin1());
        // RVT_PORT if (iio.write()) {
        if (iio.write(img)) {
            ret = true;
        }
//        QString error=iio.errorString();
    }
    QApplication::restoreOverrideCursor();

    // GraphicView deletes painter
    painter.end();
    // delete vars
    delete picture;
    delete vector;

    if (ret) {
        statusBar()->showMessage(tr("Export complete"), 2000);
    } else {
        statusBar()->showMessage(tr("Export failed!"), 2000);
    }

    return ret;
}


/**
 * Called when a sub window is about to close. 
 * If modified, show the Save/Close/Cancel dialog, then do the request.
 * If a save is needed but the user cancels, the window is not closed.
 */
void QC_ApplicationWindow::slotFileClosing(QC_MDIWindow* win)
{
    RS_DEBUG->print("QC_ApplicationWindow::slotFileClosing()");
	bool cancel = false;
	bool hasParent = win->getParentWindow() != nullptr;
	if (win && win->getDocument()->isModified() && !hasParent) {
		switch (showCloseDialog(win)) {
		case QG_ExitDialog::Save:
			cancel = !doSave(win);
			break;
		case QG_ExitDialog::Cancel:
			cancel = true;
			break;
		}
	}
	if (!cancel)
	{
		doClose(win);
		doArrangeWindows(RS2::CurrentMode);
	}
}

/**
 * File > Close All - loop through all open windows, and close them.
 * Prompt user to save changes for modified documents.  If the user cancels
 * the remaining unsaved documents will not be closed.
 *
 * @return true success
 * @return false the user cancelled.
 */
bool QC_ApplicationWindow::slotFileCloseAll()
{
	bool cancel(false), closeAll(false), hasParent(false);
	for (auto w : window_list) if (w) {

		hasParent = w->getParentWindow() != nullptr;

		if (w->getDocument()->isModified() && !hasParent && !closeAll) {
			doActivate(w);
			switch (showCloseDialog(w, window_list.count() > 1)) {
            case QG_ExitDialog::Discard:
				closeAll = true;
				break;
			case QG_ExitDialog::SaveAll:
				closeAll = slotFileSaveAll();
				break;
			case QG_ExitDialog::Save:
				cancel = !doSave(w);
				break;
			case QG_ExitDialog::Cancel:
				cancel = true;
				break;
			}
		}
		if (cancel) {
			statusBar()->showMessage(tr("Close All cancelled"), 2000);
			return false;
		}

		doClose(w);
		doArrangeWindows(RS2::CurrentMode);
	}
	return true;
}


/**
 * Menu file -> print.
 */
void QC_ApplicationWindow::slotFilePrint(bool printPDF) {
    RS_DEBUG->print(RS_Debug::D_INFORMATIONAL,"QC_ApplicationWindow::slotFilePrint(%s)", printPDF ? "PDF" : "Native");

    QC_MDIWindow* w = getMDIWindow();
    if (w==nullptr) {
        RS_DEBUG->print(RS_Debug::D_WARNING,
                "QC_ApplicationWindow::slotFilePrint: "
                "no window opened");
        return;
    }

    // Avoid printing without print preview
    if (!w->getGraphicView()->isPrintPreview())
    {
        slotFilePrintPreview(true);
        RS_DIALOGFACTORY->commandMessage(
            tr("Please adjust scaling and positioning in print preview first."));
        return;
    }

    RS_Graphic* graphic = w->getDocument()->getGraphic();
    if (graphic==nullptr) {
        RS_DEBUG->print(RS_Debug::D_WARNING,
                "QC_ApplicationWindow::slotFilePrint: "
                "no graphic");
        return;
    }

    statusBar()->showMessage(tr("Printing..."));
    QPrinter printer(QPrinter::HighResolution);

    bool landscape = false;
    RS2::PaperFormat pf = graphic->getPaperFormat(&landscape);
    QPrinter::PageSize paperSizeName = LC_Printing::rsToQtPaperFormat(pf);
    RS_Vector paperSize = graphic->getPaperSize();
    if(paperSizeName==QPrinter::Custom){
        RS_Vector s=RS_Units::convert(paperSize, graphic->getUnit(),RS2::Millimeter);
        if(landscape) s=s.flipXY();
        printer.setPageSize(QPageSize{QSizeF(s.x,s.y), QPageSize::Millimeter});
        // RS_DEBUG->print(RS_Debug::D_ERROR, "set Custom paper size to (%g, %g)\n", s.x,s.y);
    }else{
        printer.setPageSize(QPageSize{static_cast<QPageSize::PageSizeId>(paperSizeName)});
    }
    // qDebug()<<"paper size=("<<printer.paperSize(QPrinter::Millimeter).width()<<", "<<printer.paperSize(QPrinter::Millimeter).height()<<")";
    printer.setPageOrientation(landscape ? QPageLayout::Landscape : QPageLayout::Portrait);
    // margins in mm
    QMarginsF paperMargins{graphic->getMarginLeft(),
                                            graphic->getMarginRight(),
                                            graphic->getMarginTop(),
                                            graphic->getMarginBottom()};

    printMargins(paperMargins, "Drawing");
    if (printPDF) {
        // Issue #1897, exporting PDF margins to to follow the drawing settings
        QPageLayout layout = printer.pageLayout();
        layout.setMode(QPageLayout::FullPageMode);
        layout.setUnits(QPageLayout::Millimeter);
        layout.setMinimumMargins({});
        RS_Vector s=RS_Units::convert(paperSize, graphic->getUnit(),RS2::Millimeter);
        if(landscape) s=s.flipXY();
        layout.setPageSize(QPageSize{QSizeF(s.x,s.y), QPageSize::Millimeter}, paperMargins);
        printer.setPageLayout(layout);
    }
    printer.setPageMargins(paperMargins, QPageLayout::Millimeter);

    QString strDefaultFile("");
    RS_SETTINGS->beginGroup("/Print");
    strDefaultFile = RS_SETTINGS->readEntry("/FileName", "");
    printer.setOutputFileName(strDefaultFile);
    printer.setColorMode((QPrinter::ColorMode)RS_SETTINGS->readNumEntry("/ColorMode", (int)QPrinter::Color));
    RS_SETTINGS->endGroup();

    // printer setup:
    bool    bStartPrinting = false;
    if(printPDF) {
        printer.setOutputFormat(QPrinter::PdfFormat);
        printer.setColorMode(QPrinter::Color);
        printer.setResolution(1200);
        printer.setFullPage(true);
        QFileInfo   infDefaultFile(strDefaultFile);
        QFileDialog fileDlg(this, tr("Export as PDF"));
        QString     defFilter("PDF files (*.pdf)");
        QStringList filters;

        filters << defFilter
                << "Any files (*)";

        fileDlg.setNameFilters(filters);
        fileDlg.setFileMode(QFileDialog::AnyFile);
        fileDlg.selectNameFilter(defFilter);
        fileDlg.setAcceptMode(QFileDialog::AcceptSave);
        fileDlg.setDefaultSuffix("pdf");
        fileDlg.setDirectory(infDefaultFile.dir().path());
		// bug#509 setting default file name restricts selection
//        strPdfFileName = infDefaultFile.baseName();
//        if( strPdfFileName.isEmpty())
//            strPdfFileName = "unnamed";
		//fileDlg.selectFile(strPdfFileName);

        if( QDialog::Accepted == fileDlg.exec()) {
            QStringList files = fileDlg.selectedFiles();
            if (!files.isEmpty()) {
                if(!files[0].endsWith(R"(.pdf)",Qt::CaseInsensitive)) files[0]=files[0]+".pdf";
                printer.setOutputFileName(files[0]);
                bStartPrinting = true;
            }
        }
    } else {
        printer.setOutputFileName(""); // uncheck 'Print to file' checkbox
        printer.setOutputFormat(QPrinter::NativeFormat);

        QPrintDialog printDialog(&printer, this);
        printDialog.setOption(QAbstractPrintDialog::PrintToFile);
        printDialog.setOption(QAbstractPrintDialog::PrintShowPageSize);
        bStartPrinting = (QDialog::Accepted == printDialog.exec());

        // fullPage must be set to true to get full width and height
        // (without counting margins).
        printer.setFullPage(true);
        auto equalPaperSize = [&printer](const RS_Vector& v0, const RS_Vector& v1) {
            // from DPI to pixel/mm
            auto resolution = RS_Units::convert(1., RS2::Millimeter, RS2::Inch) * printer.resolution();
            // ignore difference within two pixels
            return v0.distanceTo(v1) * resolution <= 2.;
        };
        auto equalMargins = [&printer](const QMarginsF& drawingMargins) {
            QMarginsF printerMarginsPixels = printer.pageLayout().marginsPixels(printer.resolution());
            // from DPI to pixel/mm
            auto resolution = RS_Units::convert(1., RS2::Millimeter, RS2::Inch) * printer.resolution();
            // assuming drawingMargins in mm
            QMarginsF drawingMarginsPixels = drawingMargins * resolution;
            QMarginsF diff = printerMarginsPixels - drawingMarginsPixels;
            // ignore difference within two pixels
            return std::max({std::abs(diff.left()), std::abs(diff.right()), std::abs(diff.top()), std::abs(diff.bottom())}) <= 2.;
        };

        RS_Vector paperSizeMm = RS_Units::convert(paperSize, graphic->getUnit(), RS2::Millimeter);
        QMarginsF printerMargins = printer.pageLayout().margins(QPageLayout::Millimeter);
        QRectF paperRect = printer.paperRect(QPrinter::Millimeter);
        RS_Vector printerSizeMm{paperRect.width(), paperRect.height()};
        if (bStartPrinting
            && (!equalPaperSize(printerSizeMm, paperSizeMm) || !equalMargins(paperMargins))) {
            QMessageBox msgBox(this);
            msgBox.setWindowTitle("Paper settings");
            msgBox.setText("Paper size and/or margins have been changed!");
            msgBox.setInformativeText("Do you want to apply changes to current drawing?");
            msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
            msgBox.setDefaultButton(QMessageBox::Cancel);
            QString detailedText = QString("Drawing settings:\n"
                "\tsize: %1 x %2 (%3)\n"
                "\tmargins: %4, %5, %6, %7\n"
                "\n"
                "Printer settings:\n"
                "\tsize: %8 x %9 (%10)\n"
                "\tmargins: %11, %12, %13, %14\n")
                .arg(paperSize.x)
                .arg(paperSize.y)
                .arg(RS_Units::paperFormatToString(pf))
                .arg(RS_Units::convert(paperMargins.left(), RS2::Millimeter, graphic->getUnit()))
                .arg(RS_Units::convert(paperMargins.top(), RS2::Millimeter, graphic->getUnit()))
                .arg(RS_Units::convert(paperMargins.right(), RS2::Millimeter, graphic->getUnit()))
                .arg(RS_Units::convert(paperMargins.bottom(), RS2::Millimeter, graphic->getUnit()))
                .arg(RS_Units::convert(printerSizeMm.x, RS2::Millimeter, graphic->getUnit()))
                .arg(RS_Units::convert(printerSizeMm.y, RS2::Millimeter, graphic->getUnit()))
                .arg(printer.pageLayout().pageSize().name())
                .arg(RS_Units::convert(printerMargins.left(), RS2::Millimeter, graphic->getUnit()))
                .arg(RS_Units::convert(printerMargins.top(), RS2::Millimeter, graphic->getUnit()))
                .arg(RS_Units::convert(printerMargins.right(), RS2::Millimeter, graphic->getUnit()))
                .arg(RS_Units::convert(printerMargins.bottom(), RS2::Millimeter, graphic->getUnit()));
            msgBox.setDetailedText(detailedText);
            int answer = msgBox.exec();
            switch (answer) {
            case QMessageBox::Yes:
                graphic->setPaperSize(RS_Units::convert(printerSizeMm, RS2::Millimeter, graphic->getUnit()));
                graphic->setMargins(printerMargins.left(), printerMargins.top(),
                                    printerMargins.right(), printerMargins.bottom());
                break;
            case QMessageBox::No:
                break;
            case QMessageBox::Cancel:
                bStartPrinting = false;
                break;
            }
        }
    }

    if (bStartPrinting) {
        // Try to set the printer to the highest resolution
        //todo: handler printer resolution better
        if(printer.outputFormat() == QPrinter::NativeFormat ){
            //bug#3448560
            //fixme: supportedResolutions() only reports resolution of 72dpi
            //this seems to be a Qt bug up to Qt-4.7.4
            //we might be ok to keep the default resolution

//            QList<int> res=printer.supportedResolutions ();
//            if (res.size()>0)
//                printer.setResolution(res.last());
            //        for(int i=0;i<res.size();i++){
            //        std::cout<<"res.at(i)="<<res.at(i)<<std::endl;
            //        }
        }else{//pdf or postscript format
            //fixme: user should be able to set resolution output to file
            printer.setResolution(1200);
        }

        RS_DEBUG->print(RS_Debug::D_INFORMATIONAL,"QC_ApplicationWindow::slotFilePrint: resolution is %d", printer.resolution());
        QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );
        printer.setFullPage(true);

        RS_PainterQt painter(&printer);
        painter.setDrawingMode(w->getGraphicView()->getDrawingMode());

        QMarginsF margins = printer.pageLayout().margins(QPageLayout::Millimeter);
        printMargins(margins, "Printer");
        double printerFx = (double)printer.width() / printer.widthMM();
        double printerFy = (double)printer.height() / printer.heightMM();

        painter.setClipRect(margins.left() * printerFx, margins.top() * printerFy,
                            printer.width() - (margins.left() + margins.right()) * printerFx,
                            printer.height() - (margins.top() + margins.bottom()) * printerFy);

        RS_StaticGraphicView gv(printer.width(), printer.height(), &painter);
        gv.setPrinting(true);
        gv.setBorders(0,0,0,0);
        gv.setLineWidthScaling(w->getGraphicView()->getLineWidthScaling());

        double fx = printerFx * RS_Units::getFactorToMM(graphic->getUnit());
        double fy = printerFy * RS_Units::getFactorToMM(graphic->getUnit());
//RS_DEBUG->print(RS_Debug::D_ERROR, "paper size=(%d, %d)\n",
//                printer.widthMM(),printer.heightMM());

        double f = (fx+fy)/2.0;

        double scale = graphic->getPaperScale();

        gv.setFactor(f*scale);
//RS_DEBUG->print(RS_Debug::D_ERROR, "PaperSize=(%d, %d)\n",printer.widthMM(), printer.heightMM());
        gv.setContainer(graphic);

        double baseX = graphic->getPaperInsertionBase().x;
        double baseY = graphic->getPaperInsertionBase().y;
        int numX = graphic->getPagesNumHoriz();
        int numY = graphic->getPagesNumVert();
        RS_Vector printArea = graphic->getPrintAreaSize(false);

        for (int pY = 0; pY < numY; pY++) {
            double offsetY = printArea.y * pY;
            for (int pX = 0; pX < numX; pX++) {
                double offsetX = printArea.x * pX;
                // First page is created automatically.
                // Extra pages must be created manually.
                if (pX > 0 || pY > 0) printer.newPage();
                gv.setOffset((int)((baseX - offsetX) * f),
                             (int)((baseY - offsetY) * f));
//fixme, I don't understand the meaning of 'true' here
//        gv.drawEntity(&painter, graphic, true);
                painter.setDrawSelectedOnly(true);
                gv.drawEntity(&painter, graphic);
                painter.setDrawSelectedOnly(false);
                gv.drawEntity(&painter, graphic);
            }
        }

        // GraphicView deletes painter
        painter.end();

        RS_SETTINGS->beginGroup("/Print");
        RS_SETTINGS->writeEntry("/ColorMode", (int)printer.colorMode());
        RS_SETTINGS->writeEntry("/FileName", printer.outputFileName());
        RS_SETTINGS->endGroup();
        QApplication::restoreOverrideCursor();
    }

    statusBar()->showMessage(tr("Printing complete"), 2000);
}

void QC_ApplicationWindow::slotFilePrintPDF() {
    slotFilePrint(true);
}



/*	*
 *	Function name:
 *	Description:
 *	Author(s):		..., Claude Sylvain
 *	Created:			?
 *	Last modified:	30 July 2011
 *
 *	Parameters:		bool on:
 *							...
 *
 *	Returns:			void
 *	Notes:			Menu file -> print preview.
 *	*/

void QC_ApplicationWindow::slotFilePrintPreview(bool on)
{
    RS_DEBUG->print("QC_ApplicationWindow::slotFilePrintPreview()");

    QC_MDIWindow* parent = getMDIWindow();

	if (!parent)
    {
        RS_DEBUG->print(RS_Debug::D_WARNING,
                "QC_ApplicationWindow::slotFilePrintPreview: "
                "no window opened");
        return;
    }

    // close print preview:
	if (!on)
    {
        RS_DEBUG->print("QC_ApplicationWindow::slotFilePrintPreview(): off");

        if (parent->getGraphicView()->isPrintPreview())
        {
            RS_DEBUG->print("QC_ApplicationWindow::slotFilePrintPreview(): close");
            emit(printPreviewChanged(false));
            doClose(parent);
            doArrangeWindows(RS2::CurrentMode);
            return;
        }
    }

    // open print preview:
    else {
        // look for an existing print preview:
        QC_MDIWindow* ppv = parent->getPrintPreview();
        if (ppv)
        {
            RS_DEBUG->print("QC_ApplicationWindow::slotFilePrintPreview(): show existing");

            doActivate(ppv);
            doArrangeWindows(RS2::CurrentMode);
            emit(printPreviewChanged(true));
        }
        else
        {
            if (!parent->getGraphicView()->isPrintPreview())
            {
                QSettings settings;
                //generate a new print preview
                RS_DEBUG->print("QC_ApplicationWindow::slotFilePrintPreview(): create");

                QC_MDIWindow* w = new QC_MDIWindow(parent->getDocument(), mdiAreaCAD, {});
                mdiAreaCAD->addSubWindow(w);
                parent->addChildWindow(w);

                w->setWindowTitle(tr("Print preview for %1").arg(parent->windowTitle()));
                w->setWindowIcon(QIcon(":/main/document.png"));
                QG_GraphicView* gv = w->getGraphicView();
                gv->device = settings.value("Hardware/Device", "Mouse").toString();
                gv->setPrintPreview(true);
                gv->setBackground(RS_Color(255,255,255));
                gv->setDefaultAction(new RS_ActionPrintPreview(*w->getDocument(), *w->getGraphicView()));

                // only graphics offer block lists, blocks don't
                RS_DEBUG->print("  adding listeners");
                RS_Graphic* graphic = w->getDocument()->getGraphic();
                if (graphic) {
                    // Link the layer list to the pen tool bar
                    graphic->addLayerListListener(penToolBar);
                    // Link the layer list to the layer widget
                    graphic->addLayerListListener(layerWidget);
                    // link the layer list ot the layer tree widget
                    graphic->addLayerListListener(layerTreeWidget);

                    // Link the block list to the block widget
                    graphic->addBlockListListener(blockWidget);

                }

				// Link the graphic view to the mouse widget:
				QG_DIALOGFACTORY->setMouseWidget(mouseWidget);
				// Link the graphic view to the coordinate widget:
				QG_DIALOGFACTORY->setCoordinateWidget(coordinateWidget);
				QG_DIALOGFACTORY->setSelectionWidget(selectionWidget);
				// Link the graphic view to the option widget:
				//QG_DIALOGFACTORY->setOptionWidget(optionWidget);
				// Link the graphic view to the command widget:
				QG_DIALOGFACTORY->setCommandWidget(commandWidget);

                RS_DEBUG->print("  showing MDI window");

                doActivate(w);
                doArrangeWindows(RS2::CurrentMode);

                gv->zoomAuto(false);

                if(graphic){
                    bool bigger = graphic->isBiggerThanPaper();
                    bool fixed = graphic->getPaperScaleFixed();

                    graphic->fitToPage();

                    // Calling zoomPage() after fitToPage() always fits
                    // preview paper in preview window. The only reason not
                    // to call zoomPage() is when drawing is bigger than paper,
                    // plus it is fixed. In that case, not calling zoomPage()
                    // prevents displaying empty paper (when drawing is actually
                    // outside the paper and the preview window) and displays
                    // full drawing and smaller paper inside it.
                    if (bigger && fixed) {
                        RS_DEBUG->print("%s: don't call zoomPage()", __func__);
                    } else {
                        RS_DEBUG->print("%s: call zoomPage()", __func__);
                        gv->zoomPage();
                    }
                }

                emit printPreviewChanged(true);
            }
        }
    }
}





/**
 * Menu file -> quit.
 */
void QC_ApplicationWindow::slotFileQuit() {
    RS_DEBUG->print("QC_ApplicationWindow::slotFileQuit()");

    statusBar()->showMessage(tr("Exiting application..."));

    if (queryExit(false)) {
        qApp->quit();
    }
}


/**
 * Shows / hides the grid.
 *
 * @param toggle true: show, false: hide.
 */
void QC_ApplicationWindow::slotViewGrid(bool toggle) {
    RS_DEBUG->print("QC_ApplicationWindow::slotViewGrid()");

    QC_MDIWindow* m = getMDIWindow();
    if (m) {
        RS_Graphic* g = m->getGraphic();
        if (g) {
            g->setGridOn(toggle);
        }
    }

    updateGrids();
    redrawAll();

    RS_DEBUG->print("QC_ApplicationWindow::slotViewGrid() OK");
}

/**
 * Enables / disables the draft mode.
 *
 * @param toggle true: enable, false: disable.
 */
void QC_ApplicationWindow::slotViewDraft(bool toggle)
{
    RS_DEBUG->print("QC_ApplicationWindow::slotViewDraft()");

    RS_SETTINGS->beginGroup("/Appearance");
    RS_SETTINGS->writeEntry("/DraftMode", (int)toggle);
    RS_SETTINGS->endGroup();

    //handle "Draft Mode" in window titles
    QString draft_string = " ["+tr("Draft Mode")+"]";

    foreach (QC_MDIWindow* win, window_list)
    {
        win->getGraphicView()->setDraftMode(toggle);
        QString title = win->windowTitle();

        if (toggle && !title.contains(draft_string))
        {
            win->setWindowTitle(title + draft_string);
        }
        else if (!toggle && title.contains(draft_string))
        {
            title.remove(draft_string);
            win->setWindowTitle(title);
        }
    }
    redrawAll();
}

/**
 * Redraws all mdi windows.
 */
void QC_ApplicationWindow::redrawAll()
{
    if (mdiAreaCAD)
    {
        foreach (const QC_MDIWindow* win, window_list)
        {
            if (win)
            {
                QG_GraphicView* gv = win->getGraphicView();
                if (gv) {gv->redraw();}
            }
        }
    }
}



/**
 * Updates all grids of all graphic views.
 */
void QC_ApplicationWindow::updateGrids() {
    if (mdiAreaCAD) {
        QList<QMdiSubWindow*> windows = mdiAreaCAD->subWindowList();
        for (int i = 0; i < windows.size(); ++i) {
            QC_MDIWindow* m = qobject_cast<QC_MDIWindow*>(windows.at(i));
            if (m) {
                QG_GraphicView* gv = m->getGraphicView();
                if (gv) {
                    // gv->updateGrid();
                    gv->redraw(RS2::RedrawGrid);
                }
            }
        }
    }
}



/**
 * Shows / hides the status bar.
 *
 * @param toggle true: show, false: hide.
 */
void QC_ApplicationWindow::slotViewStatusBar(bool toggle) {
    RS_DEBUG->print("QC_ApplicationWindow::slotViewStatusBar()");

    statusBar()->setVisible(toggle);
}

/**
 * Shows the dialog for general application preferences.
 */
void QC_ApplicationWindow::slotOptionsGeneral() {
    RS_DIALOGFACTORY->requestOptionsGeneralDialog();

    RS_SETTINGS->beginGroup("Colors");
    QColor background(RS_SETTINGS->readEntry("/background", RS_Settings::background));
    QColor gridColor(RS_SETTINGS->readEntry("/grid", RS_Settings::grid));
    QColor metaGridColor(RS_SETTINGS->readEntry("/meta_grid", RS_Settings::meta_grid));
    QColor selectedColor(RS_SETTINGS->readEntry("/select", RS_Settings::select));
    QColor highlightedColor(RS_SETTINGS->readEntry("/highlight", RS_Settings::highlight));
    QColor startHandleColor(RS_SETTINGS->readEntry("/start_handle", RS_Settings::start_handle));
    QColor handleColor(RS_SETTINGS->readEntry("/handle", RS_Settings::handle));
    QColor endHandleColor(RS_SETTINGS->readEntry("/end_handle", RS_Settings::end_handle));
    QColor relativeZeroColor(RS_SETTINGS->readEntry("/relativeZeroColor", RS_Settings::relativeZeroColor));
    RS_SETTINGS->endGroup();

    RS_SETTINGS->beginGroup("/Appearance");
    int antialiasing = RS_SETTINGS->readNumEntry("/Antialiasing");
    bool hideRelativeZero = RS_SETTINGS->readNumEntry("/hideRelativeZero", 0) == 1;
    RS_SETTINGS->endGroup();

    emit signalEnableRelativeZeroSnaps(!hideRelativeZero);

    QList<QMdiSubWindow*> windows = mdiAreaCAD->subWindowList();
    for (int i = 0; i < windows.size(); ++i) {
        QC_MDIWindow* m = qobject_cast<QC_MDIWindow*>(windows.at(i));
        if (m) {
            QG_GraphicView* gv = m->getGraphicView();
            if (gv) {
                gv->setBackground(background);
                gv->setGridColor(gridColor);
                gv->setMetaGridColor(metaGridColor);
                gv->setSelectedColor(selectedColor);
                gv->setHighlightedColor(highlightedColor);
                gv->setStartHandleColor(startHandleColor);
                gv->setHandleColor(handleColor);
                gv->setEndHandleColor(endHandleColor);
                gv->setRelativeZeroColor(relativeZeroColor);
                gv->setRelativeZeroHiddenState(hideRelativeZero);
                gv->setAntialiasing(antialiasing);
                gv->redraw(RS2::RedrawGrid);
            }
        }
    }
}


/**
 * Menu File -> import -> importBlock
 */
void QC_ApplicationWindow::slotImportBlock() {

    if (getMDIWindow() == nullptr) {
        RS_DEBUG->print(RS_Debug::D_WARNING,
                "QC_ApplicationWindow::slotImportBlock: "
                "no window opened");
        return;
    }

    QG_FileDialog dlg(this);
    RS2::FormatType type = RS2::FormatDXFRW;
    QString dxfPath = dlg.getOpenFile(&type);
    if (dxfPath.isEmpty()) {
        return;
    }

    if (QFileInfo(dxfPath).isReadable()) {
        if (actionHandler) {
            RS_ActionInterface* a =
                actionHandler->setCurrentAction(RS2::ActionLibraryInsert);
            if (a) {
                RS_ActionLibraryInsert* action = (RS_ActionLibraryInsert*)a;
                action->setFile(dxfPath);
            } else {
                RS_DEBUG->print(RS_Debug::D_ERROR,
                                "QC_ApplicationWindow::slotImportBlock:"
                                "Cannot create action RS_ActionLibraryInsert");
            }
        }
    } else {
        RS_DEBUG->print(RS_Debug::D_ERROR,
                        "QC_ApplicationWindow::slotImportBlock: Can't read file: '%s'", dxfPath.toLatin1().data());
    }
}


void QC_ApplicationWindow::showAboutWindow()
{
    // author: ravas

    QDialog dlg;
    dlg.setWindowTitle(tr("About"));

    auto layout = new QVBoxLayout;
    dlg.setLayout(layout);

    auto frame = new QGroupBox(qApp->applicationName());
    layout->addWidget(frame);

    auto f_layout = new QVBoxLayout;
    frame->setLayout(f_layout);

    // Compiler macro list in Qt source tree
    // Src/qtbase/src/corelib/global/qcompilerdetection.h

    QString info
    (
        QStringLiteral(KUUBIK_DRAW_PRODUCT_NAME " " KUUBIK_DRAW_VERSION " \u2014 "
                       KUUBIK_DRAW_BASE_VERSION) + "\n" +
        tr("LibreCAD base version: %1").arg(XSTR(LC_VERSION)) + "\n" +
        #if defined(Q_CC_CLANG)
            tr("Compiler: Clang %1.%2.%3").arg(__clang_major__).arg(__clang_minor__).arg(__clang_patchlevel__) + "\n" +
        #elif defined(Q_CC_GNU)
            tr("Compiler: GNU GCC %1.%2.%3").arg(__GNUC__).arg(__GNUC_MINOR__).arg(__GNUC_PATCHLEVEL__) + "\n" +
        #elif defined(Q_CC_MSVC)
            tr("Compiler: Microsoft Visual C++") + "\n" +
        #endif
        tr("Compiled on: %1").arg(__DATE__) + "\n" +
        tr("Qt Version: %1").arg(qVersion()) + "\n" +
        tr("Boost Version: %1.%2.%3").arg(BOOST_VERSION / 100000).arg(BOOST_VERSION / 100 % 1000).arg(BOOST_VERSION % 100)
    );

    auto app_info = new QLabel(info);
    app_info->setTextInteractionFlags(Qt::TextSelectableByMouse);
    f_layout->addWidget(app_info);

    auto copy_button = new QPushButton(tr("Copy"));
    // copy_button->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    f_layout->addWidget(copy_button);

    connect(copy_button, SIGNAL(released()), &dlg, SLOT(accept()));

    auto attribution_label = new QLabel(
        tr("Kuubik Draw is a GPLv2 fork of LibreCAD. LibreCAD authors and upstream attribution are preserved below."));
    attribution_label->setWordWrap(true);
    f_layout->addWidget(attribution_label);

    QLabel *links_label = new QLabel( QString( "<a href=\"https://github.com/LibreCAD/LibreCAD/graphs/contributors\">%1</a>"
                                               "<br/>"
                                               "<a href=\"https://github.com/LibreCAD/LibreCAD/blob/v2.2.1.5/LICENSE\">%2</a>"
                                               "<br/>"
                                               "<a href=\"https://github.com/LibreCAD/LibreCAD/\">%3</a>"
                                               "<br/>"
                                               "<a href=\"https://github.com/T3stin-svg/kuubik-draw-native\">%4</a>")
                                      .arg( tr("LibreCAD contributors"))
                                      .arg( tr("GPLv2 license"))
                                      .arg( tr("LibreCAD upstream source"))
                                      .arg( tr("Kuubik Draw source")) );
    links_label->setOpenExternalLinks(true);
    links_label->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
    f_layout->addWidget(links_label);

    if (dlg.exec())
    {
        QClipboard* clipboard = QApplication::clipboard();
        #if QT_VERSION >= 0x050400
           info += "\n" + tr("System") + ": " + QSysInfo::prettyProductName();
        #endif
        clipboard->setText(info);
    }
}

/**
 * overloaded for Message box on last window exit.
 */
bool QC_ApplicationWindow::queryExit(bool force){
    RS_DEBUG->print("QC_ApplicationWindow::queryExit()");
    bool succ = true;
    if (force)
        for (auto w: window_list)
            doClose(w);
    else {

        RS_SETTINGS->beginGroup("/Startup");
        bool saveOpenedFiles = RS_SETTINGS->readNumEntry("/OpenLastOpenedFiles", 0) == 1;
        RS_SETTINGS->endGroup();

        QString openedFiles;
        QString activeFile = "";
        if (saveOpenedFiles){
            for (auto w: window_list) {
                QString fileName = w->getDocument() ->getFilename();
                if (activedMdiSubWindow != nullptr && activedMdiSubWindow == w){
                    activeFile = fileName;
                }
                openedFiles += fileName;
                openedFiles +=";";
            }
        }
        succ = slotFileCloseAll();

        if (succ){
            if (!openedFiles.isEmpty()){
                RS_SETTINGS->beginGroup("/Startup");
                RS_SETTINGS->writeEntry("/LastOpenFilesList", openedFiles);
                RS_SETTINGS->writeEntry("/LastOpenFilesActive", activeFile);
                RS_SETTINGS->endGroup();
            }
            storeSettings();
        }
    }

    RS_DEBUG->print("QC_ApplicationWindow::queryExit(): OK");

    return succ;
}

/**
 * Handle hotkeys. Don't let it to the default handler of Qt.
 * it will consume them also if a text field is active
 * which means it's impossible to enter a command.
 */
void QC_ApplicationWindow::keyPressEvent(QKeyEvent* e)
{
    switch (e->key()) {
    case Qt::Key_Escape:
        slotKillAllActions();
        // fall-through
    case Qt::Key_Return:
    case Qt::Key_Enter:
        slotEnter();
        e->accept();
        break;

    case Qt::Key_Plus:
    case Qt::Key_Equal:
        actionHandler->slotZoomIn();
        e->accept();
        break;

    case Qt::Key_Minus:
        actionHandler->slotZoomOut();
        e->accept();
        break;

    default:
        e->ignore();
        RS_DEBUG->print("QC_ApplicationWindow::KeyPressEvent: IGNORED");
        break;
    }

    if (e->isAccepted()) {
        RS_DEBUG->print("QC_ApplicationWindow::KeyPressEvent: Accepted");
        return;
    }

    QMainWindow::keyPressEvent(e);
}


QMdiArea const* QC_ApplicationWindow::getMdiArea() const{
    return mdiAreaCAD;
}

QMdiArea* QC_ApplicationWindow::getMdiArea(){
    return mdiAreaCAD;
}

RS_GraphicView const* QC_ApplicationWindow::getGraphicView() const{
    QC_MDIWindow const* m = getMDIWindow();
    if (m) {
        return m->getGraphicView();
    }
    return nullptr;
}

RS_GraphicView * QC_ApplicationWindow::getGraphicView() {
    QC_MDIWindow* m = getMDIWindow();
    if (m) {
        return m->getGraphicView();
    }
    return nullptr;
}

RS_Document const* QC_ApplicationWindow::getDocument() const{
    QC_MDIWindow const* m = getMDIWindow();
    if (m) {
        return m->getDocument();
    }
    return nullptr;
}

RS_Document* QC_ApplicationWindow::getDocument(){
    QC_MDIWindow* m = getMDIWindow();
    if (m) {
        return m->getDocument();
    }
    return nullptr;
}

void QC_ApplicationWindow::createNewDocument(
        const QString& fileName, RS_Document* doc) {

    slotFileNew(doc);
    if (fileName!=QString() && getDocument()) {
        getDocument()->setFilename(fileName);
    }
}

void QC_ApplicationWindow::updateWindowTitle(QWidget *w)
{
    RS_DEBUG->print("QC_ApplicationWindow::slotViewDraft()");

    RS_SETTINGS->beginGroup("/Appearance");
    bool draftMode = RS_SETTINGS->readNumEntry("/DraftMode", 0);
    RS_SETTINGS->endGroup();

    if (draftMode)
    {
        QString draft_string = " ["+tr("Draft Mode")+"]";
        if (!w->windowTitle().contains(draft_string))
            w->setWindowTitle(w->windowTitle() + draft_string);
    }
}

void QC_ApplicationWindow::relayAction(QAction* q_action)
{
    // author: ravas

    auto view = getMDIWindow()->getGraphicView();
    if (!view)
    {   // when switching back to LibreCAD from another program
        // occasionally no drawings are activated
        qWarning("relayAction: graphicView is nullptr");
        return;
    }

    view->setCurrentQAction(q_action);

    const QString commands(q_action->data().toString());
    if (!commands.isEmpty())
    {
        const QString title(q_action->text().remove("&"));
        commandWidget->appendHistory(title + " : " + commands);
    }
}

/**
 * Called by Qt after a toolbar or dockwidget right-click.
 * See QMainWindow::createPopupMenu() for more information.
 */
QMenu* QC_ApplicationWindow::createPopupMenu()
{
    // author: ravas

    QMenu* context_menu = new QMenu("Context");
    context_menu->setAttribute(Qt::WA_DeleteOnClose);

    QMenu* tb_menu = menuBar()->findChild<QMenu*>("toolbars_menu");
    QMenu* temp_tb_menu = new QMenu(tr("Toolbars"), context_menu);
    temp_tb_menu->addActions(tb_menu->actions());
    context_menu->addMenu(temp_tb_menu);

    QMenu* dw_menu = menuBar()->findChild<QMenu*>("dockwidgets_menu");
    QMenu* temp_dw_menu = new QMenu(tr("Dockwidgets"), context_menu);
    temp_dw_menu->addActions(dw_menu->actions());
    context_menu->addMenu(temp_dw_menu);

    context_menu->addAction(a_map["ViewStatusBar"]);

    return context_menu;
}

void QC_ApplicationWindow::toggleFullscreen(bool checked)
{
    // author: ravas

    checked?showFullScreen():showMaximized();
}

void QC_ApplicationWindow::hideOptions(QC_MDIWindow* win)
{
    // author: ravas

    win->getGraphicView()->getDefaultAction()->hideOptions();
}

void QC_ApplicationWindow::slotFileOpenRecent(QAction* action)
{
	RS_DEBUG->print("QC_ApplicationWindow::slotFileOpenRecent()");

    statusBar()->showMessage(tr("Opening recent file..."));
    QString fileName = action->data().toString();
    slotFileOpen(fileName, RS2::FormatUnknown);
}

/**
 * This slot manipulates the widget options dialog,
 * and reads / writes the associated settings.
 */
void QC_ApplicationWindow::widgetOptionsDialog()
{
    // author: ravas

    LC_WidgetOptionsDialog dlg;

    QSettings settings;
    settings.beginGroup("Widgets");

    int allow_style = settings.value("AllowStyle", 0).toInt();
    dlg.style_checkbox->setChecked(allow_style);
    dlg.style_combobox->addItems(QStyleFactory::keys());
    if (allow_style)
    {
        QString a_style = settings.value("Style", "").toString();
        if (!a_style.isEmpty())
        {
            int index = dlg.style_combobox->findText(a_style);
            dlg.style_combobox->setCurrentIndex(index);
        }
    }

    QString sheet_path = settings.value("StyleSheet", "").toString();
    if (!sheet_path.isEmpty() && QFile::exists(sheet_path))
        dlg.stylesheet_field->setText(sheet_path);

    int allow_theme = settings.value("AllowTheme", 0).toInt();
    dlg.theme_checkbox->setChecked(allow_theme);

    int allow_toolbar_icon_size = settings.value("AllowToolbarIconSize", 0).toInt();
    dlg.toolbar_icon_size_checkbox->setChecked(allow_toolbar_icon_size);
    int toolbar_icon_size = settings.value("ToolbarIconSize", 24).toInt();
    dlg.toolbar_icon_size_spinbox->setValue(toolbar_icon_size);

    int allow_statusbar_height = settings.value("AllowStatusbarHeight", 0).toInt();
    dlg.statusbar_height_checkbox->setChecked(allow_statusbar_height);
    int statusbar_height = settings.value("StatusbarHeight", 32).toInt();
    dlg.statusbar_height_spinbox->setValue(statusbar_height);

    int allow_statusbar_fontsize = settings.value("AllowStatusbarFontSize", 0).toInt();
    dlg.statusbar_fontsize_checkbox->setChecked(allow_statusbar_fontsize);
    int statusbar_fontsize = settings.value("StatusbarFontSize", 12).toInt();
    dlg.statusbar_fontsize_spinbox->setValue(statusbar_fontsize);

    int leftToolbarColumnsCount = settings.value("LeftToolbarColumnsCount", 5).toInt();
    dlg.left_toobar_columns_spinbox->setValue(leftToolbarColumnsCount);

    if (dlg.exec())
    {
        int allow_style = dlg.style_checkbox->isChecked();
        settings.setValue("AllowStyle", allow_style);
        if (allow_style)
        {
            QString style = dlg.style_combobox->currentText();
            settings.setValue("Style", style);
            QApplication::setStyle(QStyleFactory::create(style));
        }

        QString sheet_path = dlg.stylesheet_field->text();
        settings.setValue("StyleSheet", sheet_path);
        if (loadStyleSheet(sheet_path))
            style_sheet_path = sheet_path;

        int allow_theme = dlg.theme_checkbox->isChecked();
        settings.setValue("AllowTheme", allow_theme);

        int allow_toolbar_icon_size = dlg.toolbar_icon_size_checkbox->isChecked();
        settings.setValue("AllowToolbarIconSize", allow_toolbar_icon_size);
        if (allow_toolbar_icon_size)
        {
            int toolbar_icon_size = dlg.toolbar_icon_size_spinbox->value();
            settings.setValue("ToolbarIconSize", toolbar_icon_size);
            setIconSize(QSize(toolbar_icon_size, toolbar_icon_size));
        }

        int allow_statusbar_fontsize = dlg.statusbar_fontsize_checkbox->isChecked();
        settings.setValue("AllowStatusbarFontSize", allow_statusbar_fontsize);
        if (allow_statusbar_fontsize)
        {
            int statusbar_fontsize = dlg.statusbar_fontsize_spinbox->value();
            settings.setValue("StatusbarFontSize", statusbar_fontsize);
            QFont font;
            font.setPointSize(statusbar_fontsize);
            statusBar()->setFont(font);
        }

        int allow_statusbar_height = dlg.statusbar_height_checkbox->isChecked();
        settings.setValue("AllowStatusbarHeight", allow_statusbar_height);
        if (allow_statusbar_height)
        {
            int statusbar_height = dlg.statusbar_height_spinbox->value();
            settings.setValue("StatusbarHeight", statusbar_height);
            statusBar()->setMinimumHeight(statusbar_height);
        }

        int columnCount = dlg.left_toobar_columns_spinbox->value();
        settings.setValue("LeftToolbarColumnsCount", columnCount);
    }
    settings.endGroup();
}

/**
 * This slot modifies the commandline's title bar
 * depending on the dock area it is moved to.
 */
void QC_ApplicationWindow::modifyCommandTitleBar(Qt::DockWidgetArea area)
{
    // author: ravas

    auto* cmdDockWidget = findChild<QDockWidget*>("command_dockwidget");
    if (cmdDockWidget == nullptr) {
        return;
    }

    auto* commandWidget = static_cast<QG_CommandWidget*>(cmdDockWidget->widget());
    QAction* dockingAction = commandWidget->getDockingAction();
    bool docked = area & Qt::AllDockWidgetAreas;

    QSettings settings;
    if (settings.value("Workspace/Mode", "kuubik").toString() == "kuubik"
        && area == Qt::BottomDockWidgetArea) {
        cmdDockWidget->setWindowTitle(tr("Command"));
        dockingAction->setText(tr("Float"));
        cmdDockWidget->setFeatures(QDockWidget::NoDockWidgetFeatures);
        return;
    }

    cmdDockWidget->setWindowTitle(docked ? tr("Cmd") : tr("Command line"));
    dockingAction->setText(docked ? tr("Float") : tr("Dock", "Dock the command widget to the main window"));
    QDockWidget::DockWidgetFeatures features =
            QDockWidget::DockWidgetClosable
            | QDockWidget::DockWidgetMovable
            | QDockWidget::DockWidgetFloatable;

    if (docked) features |= QDockWidget::DockWidgetVerticalTitleBar;
    cmdDockWidget->setFeatures(features);
}

bool QC_ApplicationWindow::loadStyleSheet(QString path)
{
    // author: ravas

    if (!path.isEmpty() && QFile::exists(path))
    {
        QFile file(path);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            qApp->setStyleSheet(QString::fromLatin1(file.readAll()));
            return true;
        }
    }
    return false;
}

void QC_ApplicationWindow::reloadStyleSheet()
{
    // author: ravas

    loadStyleSheet(style_sheet_path);
}

bool QC_ApplicationWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (QEvent::FileOpen == event->type()) {
        QFileOpenEvent *openEvent = static_cast<QFileOpenEvent *>(event);
        slotFileOpen(openEvent->file(), RS2::FormatUnknown);
        return true;
    }

    return QObject::eventFilter(obj, event);
}

void QC_ApplicationWindow::updateGridStatus(const QString & status)
{
    // author: ravas

   grid_status->setBottomLabel(status);
}

void QC_ApplicationWindow::showDeviceOptions()
{
    // author: ravas

    QSettings settings;

    QDialog dlg;
    dlg.setWindowTitle(tr("Device Options"));
    auto layout = new QVBoxLayout;
    auto device_combo = new ComboBoxOption(&dlg);
    device_combo->setTitle(tr("Device"));
    device_combo->setOptionsList(QStringList({"Mouse", "Tablet", "Trackpad", "Touchscreen"}));
    device_combo->setCurrentOption(settings.value("Hardware/Device", "Mouse").toString());
    layout->addWidget(device_combo);
    dlg.setLayout(layout);
    connect(device_combo, &ComboBoxOption::optionToSave,
            this, &QC_ApplicationWindow::updateDevice);
    dlg.exec();
}

void QC_ApplicationWindow::updateDevice(QString device)
{
    // author: ravas
    QSettings settings;
    settings.setValue("Hardware/Device", device);
    foreach (auto win, window_list)
    {
        win->getGraphicView()->device = device;
    }
}

void QC_ApplicationWindow::invokeToolbarCreator()
{
    // author: ravas

    auto tb_creator = findChild<QDialog*>("Toolbar Creator");
    if (tb_creator)
    {
        tb_creator->raise();
        tb_creator->activateWindow();
        return;
    }

    auto dlg = new QDialog(this);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->setWindowTitle(tr("Toolbar Creator"));
    dlg->setObjectName("Toolbar Creator");

    auto toolbar_creator = new WidgetCreator(dlg, a_map, ag_manager->allGroups());
    toolbar_creator->addCustomWidgets("CustomToolbars");

    connect(toolbar_creator, SIGNAL(widgetToCreate(QString)),
            this, SLOT(createToolbar(QString)));
    connect(toolbar_creator, SIGNAL(widgetToDestroy(QString)),
            this, SLOT(destroyToolbar(QString)));

    auto layout = new QVBoxLayout;
    layout->addWidget(toolbar_creator);
    dlg->setLayout(layout);

    dlg->show();
}

void QC_ApplicationWindow::createToolbar(const QString& toolbar_name)
{
    // author: ravas

    QSettings settings;
    auto tb = QString("CustomToolbars/%1").arg(toolbar_name);
    auto a_list = settings.value(tb).toStringList();

    auto toolbar = findChild<QToolBar*>(toolbar_name);

    if (toolbar)
        toolbar->clear();
    else
    {
        toolbar = new QToolBar(toolbar_name, this);
        toolbar->setObjectName(toolbar_name);
        addToolBar(Qt::BottomToolBarArea, toolbar);
    }

    foreach (auto key, a_list)
    {
        toolbar->addAction(a_map[key]);
    }
}

void QC_ApplicationWindow::destroyToolbar(const QString& toolbar_name)
{
    // author: ravas

    auto toolbar = findChild<QToolBar*>(toolbar_name);
    if (toolbar) delete toolbar;
}


void QC_ApplicationWindow::invokeMenuCreator()
{
    // author: ravas

    auto menu_creator = findChild<QDialog*>("Menu Creator");
    if (menu_creator)
    {
        menu_creator->raise();
        menu_creator->activateWindow();
        return;
    }

    auto dlg = new QDialog(this);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->setWindowTitle(tr("Menu Creator"));
    auto layout = new QVBoxLayout;
    auto widget_creator = new WidgetCreator(dlg, a_map, ag_manager->allGroups(), true);
    widget_creator->addCustomWidgets("CustomMenus");

    connect(widget_creator, SIGNAL(widgetToDestroy(QString)),
            this, SLOT(destroyMenu(QString)));
    connect(widget_creator, SIGNAL(widgetToAssign(QString)),
            this, SLOT(invokeMenuAssigner(QString)));
    connect(widget_creator, SIGNAL(widgetToUpdate(QString)),
            this, SLOT(updateMenu(QString)));

    layout->addWidget(widget_creator);
    dlg->setLayout(layout);
    dlg->show();
}

void QC_ApplicationWindow::invokeMenuAssigner(const QString& menu_name)
{
    //author: ravas

    QSettings settings;
    settings.beginGroup("Activators");

    QDialog dlg;
    dlg.setWindowTitle(tr("Menu Assigner"));

    auto cb_1 = new QCheckBox("Double-Click");
    auto cb_2 = new QCheckBox("Right-Click");
    auto cb_3 = new QCheckBox("Ctrl+Right-Click");
    auto cb_4 = new QCheckBox("Shift+Right-Click");
    cb_1->setChecked(settings.value("Double-Click").toString() == menu_name);
    cb_2->setChecked(settings.value("Right-Click").toString() == menu_name);
    cb_3->setChecked(settings.value("Ctrl+Right-Click").toString() == menu_name);
    cb_4->setChecked(settings.value("Shift+Right-Click").toString() == menu_name);

    auto button_box = new QDialogButtonBox;
    button_box->setStandardButtons(QDialogButtonBox::Save|QDialogButtonBox::Cancel);

    connect(button_box, SIGNAL(accepted()), &dlg, SLOT(accept()));
    connect(button_box, SIGNAL(rejected()), &dlg, SLOT(reject()));

    auto layout = new QVBoxLayout;
    dlg.setLayout(layout);

    auto frame = new QFrame;
    layout->addWidget(frame);

    auto f_layout = new QVBoxLayout;
    frame->setLayout(f_layout);

    f_layout->addWidget(cb_1);
    f_layout->addWidget(cb_2);
    f_layout->addWidget(cb_3);
    f_layout->addWidget(cb_4);
    f_layout->addWidget(button_box);

    if (dlg.exec())
    {
        if (cb_1->isChecked())
            assignMenu("Double-Click", menu_name);
        else
            unassignMenu("Double-Click", menu_name);

        if (cb_2->isChecked())
            assignMenu("Right-Click", menu_name);
        else
            unassignMenu("Right-Click", menu_name);

        if (cb_3->isChecked())
            assignMenu("Ctrl+Right-Click", menu_name);
        else
            unassignMenu("Ctrl+Right-Click", menu_name);

        if (cb_4->isChecked())
            assignMenu("Shift+Right-Click", menu_name);
        else
            unassignMenu("Shift+Right-Click", menu_name);
    }
    settings.endGroup();
}

void QC_ApplicationWindow::unassignMenu(const QString& activator, const QString& menu_name)
{
    // author: ravas

    QSettings settings;
    settings.beginGroup("Activators");

    if (settings.value(activator).toString() == menu_name)
    {
        settings.remove(activator);
    }
    settings.endGroup();

    foreach (auto win, window_list)
    {
        auto view = win->getGraphicView();
        view->destroyMenu(activator);
    }
}

void QC_ApplicationWindow::assignMenu(const QString& activator, const QString& menu_name)
{
    // author: ravas

    QSettings settings;

    settings.beginGroup("Activators");
    settings.setValue(activator, menu_name);
    settings.endGroup();

    auto menu_key = QString("CustomMenus/%1").arg(menu_name);
    auto a_list = settings.value(menu_key).toStringList();

    foreach (auto win, window_list)
    {
        auto view = win->getGraphicView();
        auto menu = new QMenu(activator, view);
        menu->setObjectName(menu_name);
        foreach (auto key, a_list)
        {
            menu->addAction(a_map[key]);
        }
        view->setMenu(activator, menu);
    }
}

void QC_ApplicationWindow::updateMenu(const QString& menu_name)
{
    // author: ravas

    QSettings settings;

    auto menu_key = QString("CustomMenus/%1").arg(menu_name);
    auto a_list = settings.value(menu_key).toStringList();

    settings.beginGroup("Activators");
    auto activators = settings.childKeys();

    foreach (auto activator, activators)
    {
        if (settings.value(activator).toString() == menu_name)
        {
            foreach (auto win, window_list)
            {
                auto view = win->getGraphicView();
                auto menu = new QMenu(activator, view);
                menu->setObjectName(menu_name);
                foreach (auto key, a_list)
                {
                    menu->addAction(a_map[key]);
                }
                view->setMenu(activator, menu);
            }
        }
    }
}

void QC_ApplicationWindow::destroyMenu(const QString& menu_name)
{
    //author: ravas

    QSettings settings;
    settings.beginGroup("Activators");
    auto activators = settings.childKeys();

    foreach (auto activator, activators)
    {
        if (settings.value(activator).toString() == menu_name)
        {
            settings.remove(activator);
            foreach (auto win, window_list)
            {
                auto view = win->getGraphicView();
                view->destroyMenu(activator);
            }
        }
    }
    settings.endGroup();
}

void QC_ApplicationWindow::changeEvent(QEvent* event)
{
    // author: ravas
    // returning to LC via Command+Tab won't always activate a subwindow #821

    #if defined(Q_OS_OSX)
        if (event->type() == QEvent::ActivationChange)
        {
            if (isActiveWindow())
            {
                if (current_subwindow)
                    mdiAreaCAD->setActiveSubWindow(current_subwindow);
            }
            else
            {
                current_subwindow = mdiAreaCAD->currentSubWindow();
            }
        }
    #else
    Q_UNUSED( event)
    #endif
}


void QC_ApplicationWindow::invokeLicenseWindow()
{
    // author: ravas

    QDialog dlg;

    dlg.setWindowTitle(QObject::tr("License"));

    auto viewer = new TextFileViewer(&dlg);
    auto layout = new QVBoxLayout;
    layout->addWidget(viewer);
    dlg.setLayout(layout);

    viewer->addFile("readme", ":/readme.md");
    viewer->addFile("GPLv2", ":/gpl-2.0.txt");

    viewer->setFile("readme");

    dlg.exec();
}


QC_MDIWindow* QC_ApplicationWindow::getWindowWithDoc(const RS_Document* doc)
{
    QC_MDIWindow* wwd = nullptr;

    if (doc) {
        foreach (QC_MDIWindow* w, window_list) {
            if (w && w->getDocument() == doc) {
                wwd = w;
                break;
            }
        }
    }
    return wwd;
}

void QC_ApplicationWindow::activateWindowWithFile(QString& fileName)
{
    if (!fileName.isEmpty()){
            foreach (QC_MDIWindow *w, window_list) {
                if (w != nullptr){}
                RS_Document *doc = w->getDocument();
                if (doc != nullptr){
                    const QString &docFileName = doc->getFilename();
                    if (fileName == docFileName){
                        doActivate(w);
                        break;
                    }
                }
            }
    }
}

void QC_ApplicationWindow::showBlockActivated(const RS_Block *block)
{
    if (blockWidget != nullptr && block != nullptr) {
        blockWidget->activateBlock(const_cast<RS_Block*>(block));
    }
}

QAction* QC_ApplicationWindow::getAction(const QString& actionName) const
{
    if (a_map.count(actionName) == 0)
        return nullptr;
    return a_map[actionName];
}
