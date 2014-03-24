/*
 * Copyright (C) 2014 Canonical, Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3, as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranties of
 * MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Pete Woods <pete.woods@canonical.com>
 */

#include <service/SignalHandler.h>

#include <QCoreApplication>
#include <QDebug>

#include <csignal>
#include <sys/socket.h>
#include <unistd.h>

using namespace hud::service;

int SignalHandler::sigintFd[2];

int SignalHandler::sigtermFd[2];

SignalHandler::SignalHandler(QObject *parent) :
		QObject(parent) {

	if (::socketpair(AF_UNIX, SOCK_STREAM, 0, sigintFd)) {
		qFatal("Couldn't create INT socketpair");
	}
	if (::socketpair(AF_UNIX, SOCK_STREAM, 0, sigtermFd)) {
		qFatal("Couldn't create TERM socketpair");
	}

	snInt = new QSocketNotifier(sigintFd[1], QSocketNotifier::Read, this);
	connect(snInt, SIGNAL(activated(int)), this, SLOT(handleSigInt()));
	snTerm = new QSocketNotifier(sigtermFd[1], QSocketNotifier::Read, this);
	connect(snTerm, SIGNAL(activated(int)), this, SLOT(handleSigTerm()));
}

void SignalHandler::intSignalHandler(int) {
	char a = 1;
	::write(sigintFd[0], &a, sizeof(a));
}

void SignalHandler::termSignalHandler(int) {
	char a = 1;
	::write(sigtermFd[0], &a, sizeof(a));
}

int SignalHandler::setup_unix_signal_handlers() {
	struct sigaction sigint, term;

	sigint.sa_handler = SignalHandler::intSignalHandler;
	sigemptyset(&sigint.sa_mask);
	sigint.sa_flags = 0;
	sigint.sa_flags |= SA_RESTART;

	if (sigaction(SIGINT, &sigint, 0) > 0)
		return 1;

	term.sa_handler = SignalHandler::termSignalHandler;
	sigemptyset(&term.sa_mask);
	term.sa_flags |= SA_RESTART;

	if (sigaction(SIGTERM, &term, 0) > 0)
		return 2;

	return 0;
}

void SignalHandler::handleSigTerm() {
	snTerm->setEnabled(false);
	char tmp;
	::read(sigtermFd[1], &tmp, sizeof(tmp));

	QCoreApplication::exit(0);

	snTerm->setEnabled(true);
}

void SignalHandler::handleSigInt() {
	snInt->setEnabled(false);
	char tmp;
	::read(sigintFd[1], &tmp, sizeof(tmp));

	QCoreApplication::exit(0);

	snInt->setEnabled(true);
}
