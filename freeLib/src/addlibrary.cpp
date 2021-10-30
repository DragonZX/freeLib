#include <QMainWindow>
#include <QToolButton>

#include "addlibrary.h"
#include "ui_addlibrary.h"
#include "common.h"
#include "quazip/quazip/quazip.h"
#include "quazip/quazip/quazipfile.h"
#include "exportdlg.h"

AddLibrary::AddLibrary(QWidget *parent) :
    QDialog(parent,Qt::Dialog|Qt::WindowSystemMenuHint),
    ui(new Ui::AddLibrary)
{
    bLibChanged = false;
    ui->setupUi(this);

    QToolButton* tbInpx=new QToolButton(this);
    tbInpx->setFocusPolicy(Qt::NoFocus);
    tbInpx->setCursor(Qt::ArrowCursor);
    tbInpx->setText(QStringLiteral("..."));
    QHBoxLayout* layout=new QHBoxLayout(ui->inpx);
    layout->addWidget(tbInpx,0,Qt::AlignRight);
    layout->setSpacing(0);
    layout->setMargin(0);

    QToolButton* tbBooksDir=new QToolButton(this);
    tbBooksDir->setFocusPolicy(Qt::NoFocus);
    tbBooksDir->setCursor(Qt::ArrowCursor);
    tbBooksDir->setText(QStringLiteral("..."));
    layout=new QHBoxLayout(ui->BookDir);
    layout->addWidget(tbBooksDir,0,Qt::AlignRight);
    layout->setSpacing(0);
    layout->setMargin(0);

    QPalette palette = QApplication::style()->standardPalette();
    bool darkTheme = palette.color(QPalette::Window).lightness()<127;
    QString sIconsPath = QStringLiteral(":/img/icons/") + (darkTheme ?QStringLiteral("dark/") :QStringLiteral("light/"));
    ui->Add->setIcon(QIcon::fromTheme(QStringLiteral("list-add"),QIcon(sIconsPath + QStringLiteral("plus.svg"))));
    ui->Del->setIcon(QIcon::fromTheme(QStringLiteral("list-remove"),QIcon(sIconsPath + QStringLiteral("minus.svg"))));


    idCurrentLib_ = idCurrentLib;
    UpdateLibList();

    connect(tbInpx, &QAbstractButton::clicked, this, &AddLibrary::InputINPX);
    connect(tbBooksDir, &QAbstractButton::clicked, this, &AddLibrary::SelectBooksDir);
    connect(ui->btnUpdate, &QPushButton::clicked, this, [=](){this->StartImport();});
    connect(ui->btnExport, &QAbstractButton::clicked, this, &AddLibrary::ExportLib);
    connect(ui->ExistingLibs, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, [=](int idLib){this->SelectLibrary(idLib);});
    connect(ui->Del, &QAbstractButton::clicked, this, &AddLibrary::DeleteLibrary);
    connect(ui->Add, &QAbstractButton::clicked, this, &AddLibrary::Add_Library);
    connect(ui->ExistingLibs->lineEdit(), &QLineEdit::editingFinished, this, &AddLibrary::ExistingLibsChanged);
    ui->add_new->setChecked(true);

    SelectLibrary(idCurrentLib_);
//    SelectLibrary();
}

AddLibrary::~AddLibrary()
{
    delete ui;
}

void AddLibrary::Add_Library()
{
    idCurrentLib_ =-1;
    QString sNewName = tr("new");
    ui->ExistingLibs->blockSignals(true);
    ui->ExistingLibs->addItem(sNewName,-1);
    SLib lib;//{sNewName,"","",false,false};
    lib.name = sNewName;
    lib.bFirstAuthor = false;
    lib.bWoDeleted = false;
    SaveLibrary(idCurrentLib_,lib);
    ui->ExistingLibs->blockSignals(false);
    ui->ExistingLibs->setCurrentIndex(ui->ExistingLibs->count()-1);
}

void AddLibrary::LogMessage(const QString &msg)
{
    while(ui->Log->count()>100)
        delete ui->Log->takeItem(0);
    ui->Log->addItem(msg);
    ui->Log->setCurrentRow(ui->Log->count()-1);
}

void AddLibrary::InputINPX()
{
    QDir::setCurrent(QFileInfo(ui->inpx->text()).absolutePath());
    QString fileName = QFileDialog::getOpenFileName(this, tr("Add library"),QLatin1String(""),tr("Library")+" (*.inpx)");
    if(!fileName.isEmpty())
    {
        ui->inpx->setText(fileName);
        ui->BookDir->setText(QFileInfo(fileName).absolutePath());
        QuaZip uz(fileName);
        if(!uz.open(QuaZip::mdUnzip))
        {
            qDebug()<<"Error open INPX file: "<<fileName;
            return;
        }
        if(SetCurrentZipFileName(&uz,QStringLiteral("COLLECTION.INFO")))
        {
            QBuffer outbuff;
            QuaZipFile zip_file(&uz);
            zip_file.open(QIODevice::ReadOnly);
            outbuff.setData(zip_file.readAll());
            zip_file.close();
            QString sLib = QString::fromUtf8(outbuff.data().left(outbuff.data().indexOf('\n')));
            ui->ExistingLibs->setItemText(ui->ExistingLibs->currentIndex(),sLib);
        }
    }
}

void AddLibrary::SelectBooksDir()
{
    QDir::setCurrent(ui->BookDir->text());
    QString dir=QFileDialog::getExistingDirectory(this,tr("Select books directory"));
    if(!dir.isEmpty())
        ui->BookDir->setText(dir);
}

void AddLibrary::UpdateLibList()

{
    if(!db_is_open)
        return;
    bool block = ui->ExistingLibs->blockSignals(true);
    ui->ExistingLibs->clear();
    auto i = mLibs.constBegin();
    while(i!=mLibs.constEnd()){
        ui->ExistingLibs->addItem(i->name,i.key());
        ++i;
    }
    ui->ExistingLibs->blockSignals(block);
}

void AddLibrary::StartImport()
{
    SLib lib;//{ui->ExistingLibs->currentText().trimmed(),ui->BookDir->text().trimmed(),ui->inpx->text().trimmed(),
               // ui->firstAuthorOnly->isChecked(),ui->checkwoDeleted->isChecked()};
    lib.name = ui->ExistingLibs->currentText().trimmed();
    lib.sInpx = ui->inpx->text().trimmed();
    lib.path = ui->BookDir->text().trimmed();
    lib.bFirstAuthor = ui->firstAuthorOnly->isChecked();
    lib.bWoDeleted = ui->checkwoDeleted->isChecked();
    StartImport(lib);
}

void AddLibrary::StartImport(SLib &Lib)
{
    QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));

    int update_type=(ui->add_new->isChecked()?UT_NEW:ui->del_old->isChecked()?UT_DEL_AND_NEW:UT_FULL);
    SaveLibrary(idCurrentLib_,Lib);
    ui->btnUpdate->setDisabled(true);
    ui->BookDir->setDisabled(true);
    ui->inpx->setDisabled(true);
    ui->ExistingLibs->setDisabled(true);
    ui->Del->setDisabled(true);
    ui->Add->setDisabled(true);
    ui->firstAuthorOnly->setDisabled(true);
    ui->checkwoDeleted->setDisabled(true);
    ui->btnCancel->setText(tr("Break"));
    ui->update_group->hide();

    thread = new QThread;
    imp_tr=new ImportThread();
    imp_tr->start(Lib.sInpx,Lib.name,Lib.path,idCurrentLib_,update_type,false,
                  Lib.bFirstAuthor&&Lib.sInpx.isEmpty(),Lib.bWoDeleted);
    imp_tr->moveToThread(thread);
    connect(imp_tr, &ImportThread::Message, this, &AddLibrary::LogMessage);
    connect(thread, &QThread::started, imp_tr, &ImportThread::process);
    connect(imp_tr, &ImportThread::End, thread, &QThread::quit);
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);
    connect(imp_tr, &ImportThread::End, this, &AddLibrary::EndUpdate);
    connect(this, &AddLibrary::break_import, imp_tr, &ImportThread::break_import);

    thread->start();
}

void AddLibrary::AddNewLibrary(SLib &lib)
{
    if(!db_is_open)
    {
        db_is_open=openDB(true,false);
    }
    idCurrentLib_ =-1;
    StartImport(lib);
    exec();
}

void AddLibrary::SelectLibrary(int idLib)
{
    bool block = ui->ExistingLibs->blockSignals(true);
    if(idLib>=0 && mLibs.count()>0){
        for(int i=0;i<ui->ExistingLibs->count();i++){
            if(ui->ExistingLibs->itemData(i).toInt()==idCurrentLib_){
                ui->ExistingLibs->setCurrentIndex(i);
                ui->BookDir->setText(mLibs[idCurrentLib_].path);
                ui->inpx->setText(mLibs[idCurrentLib_].sInpx);
                ui->firstAuthorOnly->setChecked(mLibs[idCurrentLib_].bFirstAuthor);
                ui->checkwoDeleted->setChecked(mLibs[idCurrentLib_].bWoDeleted);
                ui->OPDS->setText(idCurrentLib_<0?QLatin1String(""):QStringLiteral("<a href=\"http://localhost:%2/opds_%1\">http://localhost:%2/opds_%1</a>")
                                                  .arg(idCurrentLib_).arg(options.nOpdsPort));
                ui->HTTP->setText(idCurrentLib_<0?QLatin1String(""):QStringLiteral("<a href=\"http://localhost:%2/http_%1\">http://localhost:%2/http_%1</a>")
                                                  .arg(idCurrentLib_).arg(options.nOpdsPort));
                break;
            }
        }
    }
    ui->Del->setDisabled(idLib<0);
    ui->ExistingLibs->setDisabled(idLib<0);
    ui->inpx->setDisabled(idLib<0);
    ui->BookDir->setDisabled(idLib<0);
    ui->btnUpdate->setDisabled(idLib<0);
    ui->ExistingLibs->blockSignals(block);
}

void AddLibrary::SelectLibrary()
{
    int nIndex = ui->ExistingLibs->currentIndex();
    QString dir,inpx;
    bool firstAuthor=false;
    bool bWoDeleted = false;
    if(nIndex>=0)
        idCurrentLib_ = ui->ExistingLibs->itemData(nIndex).toInt();
    if(idCurrentLib_>=0){
        dir = mLibs[idCurrentLib_].path;
        inpx = mLibs[idCurrentLib_].sInpx;
        firstAuthor = mLibs[idCurrentLib_].bFirstAuthor;
        bWoDeleted = mLibs[idCurrentLib_].bWoDeleted;
    }

    ui->BookDir->setText(dir);
    ui->inpx->setText(inpx);
    ui->firstAuthorOnly->setChecked(firstAuthor);
    ui->checkwoDeleted->setChecked(bWoDeleted);
    ui->Del->setDisabled(idCurrentLib_<0);
    ui->ExistingLibs->setDisabled(idCurrentLib_<0);
    ui->inpx->setDisabled(idCurrentLib_<0);
    ui->BookDir->setDisabled(idCurrentLib_<0);
    ui->btnUpdate->setDisabled(idCurrentLib_<0);
    QSettings* settings=GetSettings();
    ui->OPDS->setText(idCurrentLib_<0?QLatin1String(""):QStringLiteral("<a href=\"http://localhost:%2/opds_%1\">http://localhost:%2/opds_%1</a>").arg(idCurrentLib_).arg(options.nOpdsPort));
    ui->HTTP->setText(idCurrentLib_<0?QLatin1String(""):QStringLiteral("<a href=\"http://localhost:%2/http_%1\">http://localhost:%2/http_%1</a>").arg(idCurrentLib_).arg(options.nOpdsPort));

    settings->setValue(QStringLiteral("LibID"),idCurrentLib_);
    //idCurrentLib = idCurrentLib_;
}

void AddLibrary::SaveLibrary(int idLib, const SLib &Lib)
{
    QSqlQuery query(QSqlDatabase::database(QStringLiteral("libdb")));
    int idSaveLib = idLib;
    if(idLib<0)
    {
        LogMessage(tr("Add library"));
        bool result = query.exec(QStringLiteral("INSERT INTO lib(name,path,inpx,firstAuthor,woDeleted) values('%1','%2','%3',%4,%5)").arg(Lib.name,Lib.path,Lib.sInpx,Lib.bFirstAuthor?"1":"0",Lib.bWoDeleted?"1":"0"));
        if(!result)
            qDebug()<<query.lastError().databaseText();
        idSaveLib = query.lastInsertId().toInt();
        QSettings* settings=GetSettings();
        settings->setValue(QStringLiteral("LibID"),idLib);
        settings->sync();
    }
    else
    {
        LogMessage(tr("Update library"));
        bool result = query.exec(QStringLiteral("UPDATE Lib SET name='%1',path='%2',inpx='%3' ,firstAuthor=%4, woDeleted=%5 WHERE ID=%6").arg(Lib.name,Lib.path,Lib.sInpx,Lib.bFirstAuthor?"1":"0",Lib.bWoDeleted?"1":"0").arg(idLib));
        if(!result)
            qDebug()<<query.lastError().databaseText();

    }
    mLibs[idSaveLib] = Lib;
    idCurrentLib_ = idSaveLib;
    UpdateLibList();
    SelectLibrary(idSaveLib);
    bLibChanged = true;
}

void AddLibrary::DeleteLibrary()
{
    if(idCurrentLib_<0)
        return;

    if(QMessageBox::question(this,tr("Delete library"),tr("Delete library")+" \""+ui->ExistingLibs->currentText()+"\"",QMessageBox::Yes|QMessageBox::No,QMessageBox::No)==QMessageBox::No)
        return;

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    ClearLib(QSqlDatabase::database(QStringLiteral("libdb")),idCurrentLib_,false);
    QSqlQuery query(QSqlDatabase::database(QStringLiteral("libdb")));
    query.exec("DELETE FROM lib where ID="+QString::number(idCurrentLib_));
    mLibs.remove(idCurrentLib_);
    UpdateLibList();
    if(ui->ExistingLibs->count()>0){
        ui->ExistingLibs->setCurrentIndex(0);
        SelectLibrary();
    }
    bLibChanged = true;
    QApplication::restoreOverrideCursor();
}

void AddLibrary::EndUpdate()
{
    LogMessage(tr("Ending"));
    ui->btnUpdate->setDisabled(false);
    ui->btnCancel->setText(tr("Close"));
    ui->BookDir->setDisabled(false);
    ui->inpx->setDisabled(false);
    ui->Del->setDisabled(false);
    ui->Add->setDisabled(false);
    ui->ExistingLibs->setDisabled(false);
    ui->update_group->show();
    ui->firstAuthorOnly->setDisabled(false);
    ui->checkwoDeleted->setDisabled(false);
    bLibChanged = true;
    QApplication::restoreOverrideCursor();

}

void AddLibrary::terminateImport()
{
    emit break_import();
}

void AddLibrary::reject()
{
    if (ui->btnCancel->text()==tr("Close"))
    {
        if(idCurrentLib_!=idCurrentLib){
            bLibChanged = true;
            idCurrentLib=idCurrentLib_;
        }
        QDialog::reject();
    }
    else
    {
        terminateImport();
    }
}

void AddLibrary::ExistingLibsChanged()
{
    ui->ExistingLibs->setItemText(ui->ExistingLibs->currentIndex(),ui->ExistingLibs->lineEdit()->text());
}

void AddLibrary::ExportLib()
{
    //accept();
    QString dirName = QFileDialog::getExistingDirectory(this, tr("Select destination directory"));
    ExportDlg ed(this);
    ed.exec(idCurrentLib_,dirName);

}


