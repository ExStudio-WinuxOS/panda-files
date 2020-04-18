set(CORE_SRCS
    # gio gvfs implementations
    lib/core/vfs/fm-file.c
    lib/core/vfs/fm-file.h
    lib/core/vfs/fm-xml-file.c
    lib/core/vfs/fm-xml-file.h
    lib/core/vfs/vfs-menu.c
    lib/core/vfs/vfs-search.c
    # other legacy C code
    lib/core/legacy/fm-config.c
    lib/core/legacy/fm-app-info.c
    # core data structures
    lib/core/gobjectptr.h
    lib/core/filepath.cpp
    lib/core/iconinfo.cpp
    lib/core/mimetype.cpp
    lib/core/fileinfo.cpp
    lib/core/folder.cpp
    lib/core/folderconfig.cpp
    lib/core/filemonitor.cpp
    # i/o jobs
    lib/core/job.cpp
    lib/core/filetransferjob.cpp
    lib/core/deletejob.cpp
    lib/core/dirlistjob.cpp
    lib/core/filechangeattrjob.cpp
    lib/core/fileinfojob.cpp
    lib/core/filelinkjob.cpp
    lib/core/fileoperationjob.cpp
    lib/core/filesysteminfojob.cpp
    lib/core/job.cpp
    lib/core/totalsizejob.cpp
    lib/core/trashjob.cpp
    lib/core/untrashjob.cpp
    lib/core/thumbnailjob.cpp
    # extra desktop services
    lib/core/bookmarks.cpp
    lib/core/basicfilelauncher.cpp
    lib/core/volumemanager.cpp
    lib/core/userinfocache.cpp
    lib/core/thumbnailer.cpp
    lib/core/terminal.cpp
    lib/core/archiver.cpp
    lib/core/templates.cpp
    # custom actions
    lib/customactions/fileaction.cpp
    lib/customactions/fileactionprofile.cpp
    lib/customactions/fileactioncondition.cpp
)

set(LIB_SRCS
    ${CORE_SRCS}
    lib/libfmqt.cpp
    lib/bookmarkaction.cpp
    lib/sidepane.cpp
    lib/filelauncher.cpp
    lib/foldermodel.cpp
    lib/foldermodelitem.cpp
    lib/cachedfoldermodel.cpp
    lib/proxyfoldermodel.cpp
    lib/folderview.cpp
    lib/folderitemdelegate.cpp
    lib/createnewmenu.cpp
    lib/filemenu.cpp
    lib/foldermenu.cpp
    lib/filepropsdialog.cpp
    lib/applaunchcontext.cpp
    lib/placesview.cpp
    lib/placesmodel.cpp
    lib/placesmodelitem.cpp
    lib/dirtreeview.cpp
    lib/dirtreemodel.cpp
    lib/dirtreemodelitem.cpp
    lib/dnddest.cpp
    lib/mountoperation.cpp
    lib/mountoperationpassworddialog.cpp
    lib/mountoperationquestiondialog.cpp
    lib/fileoperation.cpp
    lib/fileoperationdialog.cpp
    lib/renamedialog.cpp
    lib/pathedit.cpp
    lib/pathbar.cpp
    lib/colorbutton.cpp
    lib/fontbutton.cpp
    lib/browsehistory.cpp
    lib/utilities.cpp
    lib/dndactionmenu.cpp
    lib/editbookmarksdialog.cpp
    lib/execfiledialog.cpp
    lib/appchoosercombobox.cpp
    lib/appmenuview.cpp
    lib/appchooserdialog.cpp
    lib/filesearchdialog.cpp
    lib/filedialog.cpp
    lib/fm-search.c # might be moved to libfm later
    lib/xdndworkaround.cpp
    lib/filedialoghelper.cpp
)

set(LIB_UIS
    lib/file-props.ui
    lib/file-operation-dialog.ui
    lib/rename-dialog.ui
    lib/mount-operation-password.ui
    lib/edit-bookmarks.ui
    lib/exec-file.ui
    lib/app-chooser-dialog.ui
    lib/filesearch.ui
    lib/filedialog.ui
)

#qt5_add_dbus_adaptor(DBUS_SRCS
#  org.panda.files.xml
#  application.h
#  Application
#  applicationadaptor
#  ApplicationAdaptor
#)

# qt5_add_dbus_adaptor() already generated the moc files. It also marked the
# files with SKIP_AUTOMOC but we still need to mark them witk SKIP_AUTOGEN.
# TODO: Check if this behaviour is a CMake bug.
# set_source_files_properties(${DBUS_SRCS} PROPERTIES SKIP_AUTOGEN ON)

set(SRCS
  main.cpp
  application.cpp
  applicationadaptor.cpp
  mainwindow.cpp
  tabbar.cpp
  tabpage.cpp
  view.cpp
  viewframe.cpp
  desktopwindow.cpp
  desktopentrydialog.cpp
  settings.cpp
  launcher.cpp
  xdgdir.cpp
)

set(SOURCES ${LIB_SRCS} ${LIB_UIS} ${SRCS})

add_executable(${TARGET_NAME} ${SOURCES})
target_link_libraries(${TARGET_NAME}
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::DBus
    Qt5::X11Extras
    ${GLIB_LIBRARIES}
    ${GLIB_GIO_LIBRARIES}
    ${GLIB_GOBJECT_LIBRARIES}
    ${GLIB_GTHREAD_LIBRARIES}
    ${MENUCACHE_LIBRARIES}
    ${XCB_LIBRARIES}
    ${EXIF_LIBRARIES}
)
target_include_directories(${TARGET_NAME}
    PRIVATE "${Qt5Gui_PRIVATE_INCLUDE_DIRS}"
        core/legacy
    PUBLIC
        "${GLIB_INCLUDE_DIRS}"
        "${GLIB_GIO_UNIX_INCLUDE_DIR}"
        "${MENUCACHE_INCLUDE_DIRS}"
        "${XCB_INCLUDE_DIRS}"
        "${EXIF_INCLUDE_DIRS}"
)

target_compile_definitions(${TARGET_NAME}
    PRIVATE "LIBFM_QT_DATA_DIR=\"\""
            "GETTEXT_PACKAGE=\"\""
    PUBLIC "QT_NO_KEYWORDS"
)

install(TARGETS ${TARGET_NAME} DESTINATION /usr/bin)
