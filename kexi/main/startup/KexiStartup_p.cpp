/* This file is part of the KDE project
   Copyright (C) 2003-2004 Jaroslaw Staniek <js@iidea.pl>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "KexiStartup_p.h"

#include <kstandarddirs.h>
#include <kprogress.h>
#include <kprocess.h>
#include <kdebug.h>
#include <klocale.h>

#include <qfileinfo.h>
#include <qdir.h>
#include <qapplication.h>

SQLite2ToSQLite3Migration::SQLite2ToSQLite3Migration(const QString& filePath)
: m_filePath(filePath)
{
	m_process = 0;
	m_dlg = 0;
	result = false;
	m_run = false;
}

SQLite2ToSQLite3Migration::~SQLite2ToSQLite3Migration()
{
	delete m_process;
	m_dlg->close();
	delete m_dlg;
}

tristate SQLite2ToSQLite3Migration::run()
{
	if (m_run)
		return false;
	m_run = true;
	const QString ksqlite2to3_app = KStandardDirs::findExe( "ksqlite2to3" );
	if (ksqlite2to3_app.isEmpty())
		return false;

	QFileInfo fi(m_filePath);
	if (fi.isSymLink()) {
		m_filePath = fi.readLink();
		fi = QFileInfo(m_filePath);
	}
	//remember permissions of m_filePath
	m_restoreStat = (0==stat(QFile::encodeName(m_filePath), &m_st));

	m_process = new KProcess(this, "process");
	*m_process << ksqlite2to3_app << m_filePath;
	m_process->setWorkingDirectory( fi.dir(true).absPath() );
	connect( m_process, SIGNAL(receivedStderr(KProcess*,char*,int)),
		this, SLOT(receivedStderr(KProcess*,char*,int)));
	connect( m_process, SIGNAL(processExited(KProcess*)), this, SLOT(processExited(KProcess*)) );
	if (!m_process->start(KProcess::NotifyOnExit, KProcess::Stderr))
		return false;

	m_dlg = new KProgressDialog(0, 0, QString::null, 
		i18n("Saving \"%1\" project file to a new \"%2\" database format...")
		.arg(QFileInfo(m_filePath).fileName()).arg("SQLite3")
	);
	m_dlg->setModal(true);
	connect(m_dlg, SIGNAL(cancelClicked()), this, SLOT(cancelClicked()));
	m_dlg->setMinimumDuration(1000);
	m_dlg->setAutoClose(true);
	m_dlg->progressBar()->setTotalSteps(100);
	m_dlg->progressBar()->setProgress(0);
	m_dlg->exec();

	if (result!=true)
		return result;

	return result;
}

void SQLite2ToSQLite3Migration::receivedStderr(KProcess *, char *buffer, int buflen)
{
	char *p = buffer;
	QCString line(80);
	for (int i=0; i<buflen; i++, p++) {
		if ((i==0 || buffer[i-1]=='\n') && buffer[i]=='%') {
			bool ok;
			int j=0;
			char *q=++p;
			++i;
			line="";
			for (;i<buflen && *p>='0' && *p<='9'; j++, i++, p++)
				line+=QChar(*p);
			--i; --p;
			int percent = line.toInt(&ok);
			if (ok && percent>=0 && percent<=100 && m_dlg->progressBar()->progress()<percent) {
//				kdDebug() << percent << endl;
				m_dlg->progressBar()->setProgress(percent);
				qApp->processEvents(100);
			}
		}
	}
}

void SQLite2ToSQLite3Migration::processExited(KProcess* process)
{
	kdDebug() << "EXIT " << process->name() << endl;

	kdDebug() << process->isRunning() << " " << process->exitStatus() << endl;
	m_dlg->close();
	result = !process->isRunning() && 0==process->exitStatus();//m_process->normalExit();
	kdDebug() << result << endl;
	if (result) {
		if (m_restoreStat) {
			//restore permissions for m_filePath
			chmod(QFile::encodeName(m_filePath), m_st.st_mode);
			chown(QFile::encodeName(m_filePath), m_st.st_uid, m_st.st_gid);
		}
	}
}

void SQLite2ToSQLite3Migration::cancelClicked()
{
	kdDebug() << result << " cancelClicked() " <<m_process->isRunning() << " " 
		<< m_process->exitStatus() << endl;
	if (!m_process->isRunning() && 0==m_process->exitStatus())
		return;
	result = cancelled;
	m_process->kill();
}

