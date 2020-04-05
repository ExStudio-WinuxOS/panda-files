#ifndef DESKTOPWINDOW_H
#define DESKTOPWINDOW_H

#include "view.h"
#include "launcher.h"
#include <unordered_map>
#include <string>

#include <QHash>
#include <QPoint>
#include <QByteArray>
#include <QScreen>
#include <xcb/xcb.h>

#include "lib/core/folder.h"

namespace Fm {
class CachedFolderModel;
class ProxyFolderModel;
class FolderViewListView;
}

class Settings;
class DesktopWindow : public View
{
    Q_OBJECT

public:
    friend class Application;

    enum WallpaperMode {
        WallpaperNone,
        WallpaperStretch,
        WallpaperFit,
        WallpaperCenter,
        WallpaperTile,
        WallpaperZoom
    };

    explicit DesktopWindow(int screenNum);
    virtual ~DesktopWindow();

    void setForeground(const QColor& color);
    void setShadow(const QColor& color);
    void setBackground(const QColor& color);
    void setDesktopFolder();
    void setWallpaperFile(QString filename);
    void setWallpaperMode(WallpaperMode mode = WallpaperStretch);
    void setLastSlide(QString filename);
    void setWallpaperDir(QString dirname);
    void setSlideShowInterval(int interval);
    void setWallpaperRandomize(bool randomize);

    // void setWallpaperAlpha(qreal alpha);
    void updateWallpaper();
    bool pickWallpaper();
    void nextWallpaper();
    void updateFromSettings(Settings& settings, bool changeSlide = true);

    void queueRelayout(int delay = 0);

    int screenNum() const {
        return screenNum_;
    }

    void setScreenNum(int num);
    void increaseIconSize();
    void decreaseIconSize();

    QScreen* getDesktopScreen() const;

protected:
    virtual void prepareFolderMenu(Fm::FolderMenu* menu) override;
    virtual void prepareFileMenu(Fm::FileMenu* menu) override;
    virtual void resizeEvent(QResizeEvent* event) override;
    virtual void wheelEvent(QWheelEvent *event) override;
    virtual void onFileClicked(int type, const std::shared_ptr<const Fm::FileInfo>& fileInfo) override;

    void loadItemPositions();
    void saveItemPositions();

    QImage loadWallpaperFile(QSize requiredSize);

    virtual bool event(QEvent* event) override;
    virtual bool eventFilter(QObject* watched, QEvent* event) override;

    virtual void childDragMoveEvent(QDragMoveEvent* e) override;
    virtual void childDropEvent(QDropEvent* e) override;
    virtual void closeEvent(QCloseEvent* event) override;
    virtual void paintEvent(QPaintEvent* event) override;

protected Q_SLOTS:
    void onDesktopPreferences();
    void onCreatingShortcut();
    void selectAll();
    void toggleDesktop();

    void onRowsAboutToBeRemoved(const QModelIndex& parent, int start, int end);
    void onRowsInserted(const QModelIndex& parent, int start, int end);
    void onLayoutChanged();
    void onModelSortFilterChanged();
    void onDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight);
    void onFolderStartLoading();
    void onFolderFinishLoading();
    void onFilesAdded(const Fm::FileInfoList files);

    void relayoutItems();
    void onStickToCurrentPos(bool toggled);

    // void updateWorkArea();

    // file operations
    void onCutActivated();
    void onCopyActivated();
    void onCopyFullPathActivated();
    void onPasteActivated();
    void onRenameActivated();
    void onBulkRenameActivated();
    void onDeleteActivated();
    void onFilePropertiesActivated();

    void updateTrashIcon();

private:
    void removeBottomGap();
    void addDesktopActions(QMenu* menu);
    void paintBackground(QPaintEvent* event);
    void paintDropIndicator();
    bool stickToPosition(const std::string& file, QPoint& pos, const QRect& workArea, const QSize& grid, bool reachedLastCell = false);
    static void alignToGrid(QPoint& pos, const QPoint& topLeft, const QSize& grid, const int spacing);

    void updateShortcutsFromSettings(Settings& settings);
    void createTrashShortcut(int items);
    void createHomeShortcut();
    void createComputerShortcut();
    void createNetworkShortcut();

    void createTrash();
    static void onTrashChanged(GFileMonitor* monitor, GFile* gf, GFile* other, GFileMonitorEvent evt, DesktopWindow* pThis);
    void trustOurDesktopShortcut(std::shared_ptr<const Fm::FileInfo> file);
    bool isTrashCan(std::shared_ptr<const Fm::FileInfo> file) const;

    QImage getWallpaperImage() const;

    QModelIndex indexForPos(bool* isTrash, const QPoint& pos, const QRect& workArea, const QSize& grid) const;

private:
    Fm::ProxyFolderModel* proxyModel_;
    Fm::FolderModel* model_;
    std::shared_ptr<Fm::Folder> folder_;
    Fm::FolderViewListView* listView_;

    QColor fgColor_;
    QColor bgColor_;
    QColor shadowColor_;
    QString wallpaperFile_;
    WallpaperMode wallpaperMode_;
    QString lastSlide_;
    QString wallpaperDir_;
    int slideShowInterval_;
    QTimer* wallpaperTimer_;
    bool wallpaperRandomize_;
    QPixmap wallpaperPixmap_;
    Launcher fileLauncher_;
    bool desktopHideItems_;

    int screenNum_;
    std::unordered_map<std::string, QPoint> customItemPos_;
    QHash<QModelIndex, QString> displayNames_; // only for desktop entries and shortcuts
    QTimer* relayoutTimer_;
    QTimer* selectionTimer_;

    QRect dropRect_;

    QTimer* trashUpdateTimer_;
    GFileMonitor* trashMonitor_;

    QStringList filesToTrust_;

    int minIconSize_ = 48;
    int maxIconSize_ = 96;
};

#endif // DESKTOPWINDOW_H
