#ifndef TABPAGE_H
#define TABPAGE_H

#include <QWidget>
#include <QVBoxLayout>
#include <QLineEdit>
#include "view.h"
#include "settings.h"

#include "lib/browsehistory.h"
#include "lib/core/fileinfo.h"
#include "lib/core/filepath.h"
#include "lib/core/folder.h"

namespace Fm {
class FileLauncher;
class FolderModel;
class ProxyFolderModel;
class CachedFolderModel;
}

class Launcher;

class ProxyFilter : public Fm::ProxyFolderModelFilter {
public:
    bool filterAcceptsRow(const Fm::ProxyFolderModel* model, const std::shared_ptr<const Fm::FileInfo>& info) const;
    virtual ~ProxyFilter() {}
    QString getFilterStr() {
        return filterStr_;
    }
    void setFilterStr(QString str) {
        filterStr_ = str;
    }

private:
    QString filterStr_;
};

//==================================================

class FilterEdit : public QLineEdit {
    Q_OBJECT
public:
    FilterEdit(QWidget *parent = nullptr);
    ~FilterEdit() {};
    void keyPressed(QKeyEvent* event);

protected:
    virtual void focusOutEvent(QFocusEvent* event) override {
        Q_EMIT lostFocus();
        QLineEdit::focusOutEvent(event);
    }
    virtual void keyPressEvent(QKeyEvent* event) override;

Q_SIGNALS:
    void lostFocus();
};

class FilterBar : public QWidget {
    Q_OBJECT
public:
    FilterBar(QWidget *parent = nullptr);
    ~FilterBar() {};

    void focusBar() {
        filterEdit_->setFocus();
    }
    void clear() {
        filterEdit_->clear();
    }
    void keyPressed(QKeyEvent* event) {
        filterEdit_->keyPressed(event);
    }

Q_SIGNALS:
    void textChanged(const QString &text);
    void lostFocus();

private:
    FilterEdit* filterEdit_;
};

//==================================================

class TabPage : public QWidget {
    Q_OBJECT

public:
    enum StatusTextType {
        StatusTextNormal,
        StatusTextSelectedFiles,
        StatusTextFSInfo,
        StatusTextNum
    };

public:
    explicit TabPage(QWidget* parent = nullptr);
    virtual ~TabPage();

    void chdir(Fm::FilePath newPath, bool addHistory = true);

    Fm::FolderView::ViewMode viewMode() {
        return folderSettings_.viewMode();
    }

    void setViewMode(Fm::FolderView::ViewMode mode);

    void sort(int col, Qt::SortOrder order = Qt::AscendingOrder);

    int sortColumn() {
        return folderSettings_.sortColumn();
    }

    Qt::SortOrder sortOrder() {
        return folderSettings_.sortOrder();
    }

    bool sortFolderFirst() {
        return folderSettings_.sortFolderFirst();
    }
    void setSortFolderFirst(bool value);

    bool sortHiddenLast() {
        return folderSettings_.sortHiddenLast();
    }
    void setSortHiddenLast(bool value);

    bool sortCaseSensitive() {
        return folderSettings_.sortCaseSensitive();
    }

    void setSortCaseSensitive(bool value);

    bool showHidden() {
        return proxyModel_->showHidden();
    }

    void setShowHidden(bool showHidden);

    void setShowThumbnails(bool showThumbnails);

    void saveFolderSorting();

    Fm::FilePath path() {
        return folder_ ? folder_->path() : Fm::FilePath();
    }

    QString pathName();

    const std::shared_ptr<Fm::Folder>& folder() {
        return folder_;
    }

    Fm::FolderModel* folderModel() {
        return reinterpret_cast<Fm::FolderModel*>(folderModel_);
    }

    View* folderView() {
        return folderView_;
    }

    Fm::BrowseHistory& browseHistory() {
        return history_;
    }

    Fm::FileInfoList selectedFiles() {
        return folderView_->selectedFiles();
    }

    Fm::FilePathList selectedFilePaths() {
        return folderView_->selectedFilePaths();
    }

    void selectAll();

    void invertSelection();

    void reload();

    QString statusText(StatusTextType type = StatusTextNormal) const {
        return statusText_[type];
    }

    bool canBackward() {
        return history_.canBackward();
    }

    void backward();

    bool canForward() {
        return history_.canForward();
    }

    void forward();

    void jumpToHistory(int index);

    bool canUp();

    void up();

    void updateFromSettings(Settings& settings);

    void setFileLauncher(Fm::FileLauncher* launcher) {
        folderView_->setFileLauncher(launcher);
    }

    Fm::FileLauncher* fileLauncher() {
        return folderView_->fileLauncher();
    }

    QString getFilterStr() {
        if(proxyFilter_) {
            return proxyFilter_->getFilterStr();
        }
        return QString();
    }

    void setFilterStr(QString str) {
        if(proxyFilter_) {
            proxyFilter_->setFilterStr(str);
        }
    }

    void applyFilter();

    bool hasCustomizedView() {
        return folderSettings_.isCustomized();
    }

    void setCustomizedView(bool value);

    void transientFilterBar(bool transient);

    void showFilterBar();
    bool isFilterBarVisible() const {
        return (filterBar_ && filterBar_->isVisible());
    }
    void clearFilter() {
        if(filterBar_) {
            filterBar_->clear();
        }
    }

    void backspacePressed();

Q_SIGNALS:
    void statusChanged(int type, QString statusText);
    void titleChanged(QString title);
    void sortFilterChanged();
    void forwardRequested();
    void backwardRequested();
    void folderUnmounted();

protected:
    virtual bool eventFilter(QObject* watched, QEvent* event);

protected Q_SLOTS:
    void onSelChanged();
    void onUiUpdated();
    void onFileSizeChanged(const QModelIndex& index);
    void onFilesAdded(const Fm::FileInfoList files);
    void onFilterStringChanged(QString str);
    void onLosingFilterBarFocus();

private:
    void freeFolder();
    QString formatStatusText();

    // Adds bidi marks (RLM/LRM/RLE/LRE/POP) around the text for the statusbar.
    QString encloseWithBidiMarks(const QString& text);

    void onFolderStartLoading();
    void onFolderFinishLoading();

    // FIXME: this API design is bad and might be removed later
    void onFolderError(const Fm::GErrorPtr& err, Fm::Job::ErrorSeverity severity, Fm::Job::ErrorAction& response);

    void onFolderFsInfo();
    void onFolderRemoved();
    void onFolderUnmount();
    void onFolderContentChanged();

private:
    View* folderView_;
    Fm::CachedFolderModel* folderModel_;
    Fm::ProxyFolderModel* proxyModel_;
    ProxyFilter* proxyFilter_;
    QVBoxLayout* verticalLayout;
    std::shared_ptr<Fm::Folder> folder_;
    QString statusText_[StatusTextNum];
    Fm::BrowseHistory history_; // browsing history
    Fm::FilePath lastFolderPath_; // last browsed folder
    bool overrideCursor_;
    FolderSettings folderSettings_;
    QTimer* selectionTimer_;
    FilterBar* filterBar_;
};

#endif // TABPAGE_H