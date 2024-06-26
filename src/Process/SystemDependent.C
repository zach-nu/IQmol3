/*******************************************************************************
       
  Copyright (C) 2022 Andrew Gilbert
           
  This file is part of IQmol, a free molecular visualization program. See
  <http://iqmol.org> for more details.
       
  IQmol is free software: you can redistribute it and/or modify it under the
  terms of the GNU General Public License as published by the Free Software
  Foundation, either version 3 of the License, or (at your option) any later
  version.

  IQmol is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
  details.
      
  You should have received a copy of the GNU General Public License along
  with IQmol.  If not, see <http://www.gnu.org/licenses/>.  
   
********************************************************************************/

#include "QtVersionHacks.h"
#include "SystemDependent.h"
#include <QFileInfo>
#include <QProcess>
#include <QStringList>
#include <QMessageBox>
#include <QMap>
#include <QtDebug>
#include <QRegularExpression>

#ifdef Q_OS_WIN
#include <windows.h>
#endif


namespace GetPIDs {
   std::list<unsigned int> getpids(unsigned int);
}

namespace IQmol {
namespace System {

QStringList RunCommand(QString const& command, QStringList const& arguments, 
   unsigned int waitTimeInMilliSeconds) 
{
   QStringList output;
   QFileInfo cmd(command);

   if (cmd.exists()) {
      qDebug() << "Executing local command" << command << "with args:" << arguments;
               
      QProcess process;
      process.setWorkingDirectory(cmd.absolutePath());
      process.start(cmd.fileName(), arguments);

      if (process.waitForFinished(waitTimeInMilliSeconds)) {
         QString psOutput(process.readAllStandardOutput());
         output = psOutput.split(QRegularExpression("\\n"));
      }else {
         // should throw an exception
         process.kill();
      }

   }else {
      QString msg("Could not find command ");
      msg += command;
      QMessageBox::warning(0, "IQmol", msg);
   }

   return output;
}



QString QueryCommand(bool const local)
{
   // Expected output is a single line:
   // vi 19573   0:00.74
   QString cmd("/bin/ps -o command=,pid=,time= ${JOB_ID}");

   if (local) {
      // The command only changes for Windows boxes 
#ifdef Q_OS_WIN
      QFileInfo tasklist("C:/Windows/System32/tasklist.exe");
      if (tasklist.exists()) {
         QString spid("\"PID eq ${JOB_ID}\"");
         QStringList args;
         args << "/v" << "/fo list" << "/fi " + spid;
         cmd = tasklist.filePath() + " " + args.join(" ");
      }else {
         QString err("Error: ");
         err += tasklist.filePath() + " not found";
         return err;
      }
#endif

      // ...and OS X
#ifdef Q_OS_MAC
   cmd = "/bin/ps xc -S -o command=,pid=,time= ${JOB_ID}";
#endif
   }

   return cmd;
}


QString KillCommand(bool const local)
{
   QString cmd("/bin/kill -TERM -${JOB_ID}");
   if (local) {
#ifdef Q_OS_WIN
      QFileInfo taskkill("C:/Windows/System32/taskkill.exe");
      QFileInfo tskill("C:/Windows/System32/tskill.exe");
      if (taskkill.exists()) {       // Vista
         cmd = taskkill.filePath() + " /f /pid ${JOB_ID}";
      }else if (tskill.exists()) {   // XP
         cmd = tskill.filePath() + " ${JOB_ID}";
      }else {
         cmd = "Error: taskkill.exe or tskill.exe could not be found";
      }
#endif
   }
   return cmd;
}


QString SubmitCommand(bool const local)
{
   QString cmd("/bin/bash ${JOB_NAME}.run");
   if (local) {
#ifdef Q_OS_WIN
      cmd = "${JOB_DIR}/${JOB_NAME}.bat";
#endif
   }
   return cmd;
}


QString JobFileListCommand(bool const local)
{
   QString cmd("find ${JOB_DIR} -type f");
   if (local) {
      cmd = "(unused)";
   }
   return cmd;
}




QString TemplateForRunFile(bool const local)
{
   QString cmd;
   cmd = "#! /bin/bash\n"
         "# --- Q-Chem environment variable setup\n"
         "# . $HOME/qchem/qcenv.sh\n"
         "\n"
         "qchem ${JOB_NAME}.inp ${JOB_NAME}.out &\n"
         "echo 'JobId:' $!\n"
         "sleep 5\n";

   if (local) {
#ifdef Q_OS_WIN
      cmd = "@echo off\n"
            ":: The following variables MUST be set with correct values:\n"
            "::\n"
            ":: QC - The Q-Chem top directory.  It should contain the following\n"
            "::      files/directories after initial installation:\n"
            "::      exe, qcaux, qcref, samples, qcenv.bat, qc.bat, readme.txt\n"
            ":: QCSCRATCH - the full path of the directory for scratch files.\n"
            "::\n"
            ":: The values below are examples only, you should set your own values.\n"
            "\n"
            ":: <-- Start user configuration -->\n"
            "set QC=C:\\QChem\\5.4.0\n"
            ":: <-- End user configuration -->\n"
            "\n"
            "start /b %QC%\\qcenv ${JOB_NAME}.inp ${JOB_NAME}.out 1> ${JOB_NAME}.err 2>&1\n"
            "exit \n";
#endif
   }
   return cmd;
}


QString ExecutableName(bool const local)
{
   QString cmd("qchem.exe");
   if (local) {
#ifdef Q_OS_WIN
      //cmd = "qchem_s.exe";
#endif
   }
   return cmd;
}




unsigned int ExecutablePid(QString const& processName, QProcess const& parent) 
{
   // The strategy here is:
   //  (1) get a list of all PIDs that match processName
   //  (2) get the parentage of each of these
   //  (3) find the one who has a parent that matches the QProcess::pid()
   unsigned int qpid(ProcessID(parent));
   QList<unsigned int> ancestry;
   QList<unsigned int> pids(GetMatchingProcessIds(processName));
   QList<unsigned int>::iterator iter, jter;

   qDebug() << "Parent QProcess id:" << qpid;

   for (iter = pids.begin(); iter != pids.end(); ++iter) {
       qDebug() << processName << "process found on pid:" << *iter;
       ancestry = GetParentProcessChain(*iter);

       if (ancestry.contains(qpid)) {
          qDebug() << "Found child process" << processName << "on PID" << *iter;
          for (jter = ancestry.begin(); jter != ancestry.end(); ++jter) {
              if (*jter == qpid) {
                 qDebug() << "  ->" << *jter << "* (QProcess)";
              }else {
                 qDebug() << "  ->" << *jter;
              }
          }
          
		  // return the PID for the spawning script in case there are multiple
		  // jobs in the input file
#ifdef Q_OS_WIN
          if (ancestry.size() > 1) return ancestry[0];
#else
          if (ancestry.size() > 1) return ancestry[1];
#endif
          return *iter;
       }
   }

   return 0;
}


int ProcessID(QProcess const& process)
{
#ifdef Q_OS_WIN
   _PROCESS_INFORMATION* pi(process.pid());
   if (pi) {
      return pi->dwProcessId;
   }else {
      return 0;
   }
#else
   return process.processId();
#endif
}


QList<unsigned int> GetMatchingProcessIds(QString const& pattern) 
{
   QList<unsigned int> pids;
   QStringList args;

#ifdef Q_OS_WIN
   QFileInfo cmdInfo("C:/Windows/System32/tasklist.exe");
#else
   QFileInfo cmdInfo("/bin/ps");
   // The command,pid ordering is chosen to match the output of tasklist
   args << "-x" << "-c" << "-o" << "command,pid";
#endif
   QString cmd(cmdInfo.absoluteFilePath());

   QStringList processes(RunCommand(cmd, args));
   QStringList tokens;

   for (int i = 0; i < processes.count(); ++i) {
       if (processes[i].contains(pattern, Qt::CaseInsensitive)) {
qDebug() << "MATCHED!" << processes[i];
          tokens = processes[i].split(QRegularExpression("\\s+"), IQmolSkipEmptyParts);
          pids.append(tokens[1].toUInt());
       }
   }
   
   return pids;
}


QList<unsigned int> GetParentProcessChain(unsigned int const pid) 
{
#ifdef Q_OS_WIN
  return QList<unsigned int>::fromStdList(GetPIDs::getpids(pid));
#else
   QList<unsigned int> ancestry;

   QString psCommand("/bin/ps");
   QStringList args;
   args << "xww" << "-o" << "pid,ppid";

   QStringList lines(RunCommand(psCommand, args));
   QStringList tokens;

   QMap<unsigned int,unsigned int> psMap;
   unsigned int child, parent;
   bool isUInt0, isUInt1;

   for (int i = 0; i < lines.size(); ++i) {
       tokens = lines[i].split(QRegularExpression("\\s+"), IQmolSkipEmptyParts);
       if (tokens.count() == 2) {
          child  = tokens[0].toUInt(&isUInt0);
          parent = tokens[1].toUInt(&isUInt1);
          if (isUInt0 && isUInt1) {
             psMap[child] = parent;
          }else if (tokens[0].toLower() == "pid") {
             qDebug() << "Bad PID in GetParentProcessChain:" 
                       << tokens[0] << tokens[1];
          }
       }else {
          //qDebug() << "Incorrect number of tokens from ps"
          //         << "command in GetParentProcessChain:" << lines[i];
       }
   }

   QMap<unsigned int,unsigned int>::iterator iter; 
   ancestry.append(pid);
   child = pid;
      
   for (;;) {
       iter = psMap.find(child); 
       if (iter == psMap.end()) break;
       parent = iter.value();
       ancestry.append(parent);
       child = parent;
   }

   return ancestry;
#endif
}

} } // end namespace IQmol::Process
