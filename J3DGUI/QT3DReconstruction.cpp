#include "QT3DReconstruction.h"


QT3DReconstruction::QT3DReconstruction(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	
	QTimer *timer = new QTimer(this); //this 为parent类, 表示当前窗口

	connect(timer, SIGNAL(timeout()), this, SLOT(timerSlot()));

	timer->start(500); // 1000毫秒, 等于 1 秒
	

}


void QT3DReconstruction::on_actionMatchFeature_triggered()
{
	dlgmf.exec();
}

void QT3DReconstruction::on_actionSFM_triggered()
{
	dlgsfm.exec();
}


LPCWSTR QT3DReconstruction::LPSTRtoLPCWSTR(char* szStr)
{
	WCHAR wszClassName[256];
	memset(wszClassName, 0, sizeof(wszClassName));
	MultiByteToWideChar(CP_ACP, 0, szStr, strlen(szStr) + 1, wszClassName,
		sizeof(wszClassName) / sizeof(wszClassName[0]));
	return wszClassName;
}


void QT3DReconstruction::on_action_viewMVS_triggered()
{

}

void QT3DReconstruction::on_action_addSensorWidth_triggered()
{
	bool isok;
    QString text = QInputDialog::getText(this, "输入相机参数", "请输入对应值（格式 model;width） ", QLineEdit::Normal, "AAA;10", &isok);
	if (isok)
	{
		QFile swd("SenWidDB.txt");
		if (swd.open(QIODevice::ReadWrite | QIODevice::Append | QIODevice::Text))
		{
			swd.write("\n");
			swd.write(text.toUtf8());
			swd.close();
            QMessageBox::information(this, "成功", "成功输入相机参数 ", QMessageBox::Ok, QMessageBox::Ok);
		}
		else
		{
            QMessageBox::information(this, "失败", "无法打开SensorWidthDB文件 ", QMessageBox::Ok, QMessageBox::Ok);
		}

	}
}

void QT3DReconstruction::on_action_viewPLY_triggered()
{
	QString fileName = QFileDialog::getOpenFileName(NULL, "ViewPLY", ".", "*.ply");
	if (fileName == "")
		return;
	//QMessageBox::information(NULL, "", filename, QMessageBox::Yes, NULL);

	QByteArray buf = fileName.toLatin1(); // must
	//this->ply->~PlyIO();
	this->ply = new PlyIO(buf.data());
	ui.J3DViewerWidget->ply = this->ply;
	if (!this->ply->open()) {
		ui.textBrowser->insertPlainText("\n模型加载失败");
		ui.textBrowser->update();
	}
	


	//QProcess myProcess(this);
	//QString program = "J3DViewer.exe";
	//QStringList arguments;
	//arguments << fileName;
	//myProcess.startDetached(program, arguments, NULL, NULL);

}

void QT3DReconstruction::on_actionDenseCloud_triggered()
{
	dlgdense.exec();
}

void QT3DReconstruction::on_action_reconstrctMesh_triggered()
{
	dlgrm.exec();
}

void QT3DReconstruction::timerSlot()
{
	
    if (Global::GetProcessidFromName("J3DEngine.exe") != 0 )
	{
		Global::connectEngine();
        if (ui.label_engine->text() != "成功连接到J3DEngine ")
		{
			ui.textBrowser->insertPlainText("与J3DEngine成功建立连接\n");
			QPalette pa;
			pa.setColor(QPalette::WindowText, Qt::green);
			ui.label_engine->setPalette(pa);
            ui.label_engine->setText("成功连接到J3DEngine ");
		}	
	}
	else
	{
        if (ui.label_engine->text() != "未连接到J3DEngine ")
		{
			ui.textBrowser->insertPlainText("与J3DEngine失去连接，之后将重新尝试连接\n");
			QPalette pa;
			pa.setColor(QPalette::WindowText, Qt::red);
			ui.label_engine->setPalette(pa);
            ui.label_engine->setText("未连接到J3DEngine ");
		}
	}
	
	
	if (Global::tasking == true)
	{
		Global::getProcessMsg();
		qDebug("%d %d %d", Global::process, Global::processProject, Global::processState);
		if (Global::process == PROCESSCLOSE)
		{
			ifstream cmdCache;
			cmdCache.open(("C:\\ProgramData\\J3DEngine\\cmdCache.tmp"), ios::in | ios::_Nocreate);
			std::string temp;
			getline(cmdCache, temp);
			if (temp == "densifypointcloud") {
				QString fileName = Global::densifyWorkingDir + "/DenseCloud.ply";
				QByteArray buf = fileName.toLatin1();
				this->ply = new PlyIO(buf.data());
				ui.J3DViewerWidget->ply = this->ply;
				this->ply->open();
			}
			else if (temp == "reconstructmesh") {
				QString fileName = Global::reconstructMeshWorkingDir + "/TIN_Mesh.ply";
				QByteArray buf = fileName.toLatin1();
				qDebug("%s", buf.data());
				this->ply = new PlyIO(buf.data());
				ui.J3DViewerWidget->ply = this->ply;
				this->ply->open();
			}
            QMessageBox::information(NULL, "完成", "任务完成！ ", QMessageBox::Ok, QMessageBox::Ok);
			Global::tasking = false;
			ui.label_process->setText("等待任务 ");
			ui.progressBar->setValue(0);
			return;
		}
		else if (Global::process == PROCESSERROR)
		{
            QMessageBox::information(NULL, "失败", "任务失败，具体任务日志于J3DEngine查询 ", QMessageBox::Ok, QMessageBox::Ok);
			Global::tasking = false;
            ui.label_process->setText("等待任务 ");
			ui.progressBar->setValue(0);
			return;
		}
		else if (Global::processProject < 1)
		{
            ui.label_process->setText("等待任务 ");
			ui.progressBar->setValue(0);
			return;
		}
		else
		{

			switch (Global::processProject)
			{
			case LISTIMAGES:
			{
                ui.label_process->setText("任务进行中：加载图片 ");
				ui.progressBar->setValue(Global::processState);
				break;
			}
			case COMPUTEFEATURES:
			{
                ui.label_process->setText("任务进行中：计算特征点 ");
				ui.progressBar->setValue(Global::processState);
				break;
			}
			case MATCHFEATURES:
			{
                ui.label_process->setText("任务进行中：匹配特征点 ");
				ui.progressBar->setValue(Global::processState);
				break;
			}
			case SFM:
			{
                ui.label_process->setText("任务进行中：三维重建_SFM ");
				ui.progressBar->setValue(Global::processState);
				break;
			}
			case SFP:
			{
                ui.label_process->setText("任务进行中：三维重建_SFP ");
				ui.progressBar->setValue(Global::processState);
				break;
			}
			case COLORED:
			{
                ui.label_process->setText("任务进行中：点云上色 ");
				ui.progressBar->setValue(Global::processState);
				break;
			}
			case SPARSE:
			{
                ui.label_process->setText("任务进行中：生成稀疏点云 ");
				ui.progressBar->setValue(Global::processState);
				break;
			}
			case DENSE:
			{
                ui.label_process->setText("任务进行中：正在重建模型 ");
				ui.progressBar->setValue(Global::processState);
				break;
			}
			case REMESH:
			{
				ui.label_process->setText("任务进行中：生成三角网模型 ");
				ui.progressBar->setValue(Global::processState);
				break;
			}
			default:
			{
                ui.label_process->setText("等待任务 ");
				ui.progressBar->setValue(0);
				break;
			}
				
			}
			
			
		}
	}
}
