/*
 * Copyright (C) 2013 Canonical, Ltd.
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

#include <service/Factory.h>
#include <service/WindowContextImpl.h>
#include <QDebug>

using namespace hud::service;

WindowContextImpl::WindowContextImpl(Factory &factory) {
}

WindowContextImpl::~WindowContextImpl() {
}

void WindowContextImpl::setContext(const QString &context) {
	m_context = context;
	qDebug() << "WindowContextImpl::setContext" << context;
}

void WindowContextImpl::addAction(const QString &context, const QString &prefix,
		const QDBusObjectPath &path) {
	qDebug() << "WindowContextImpl::addAction" << context << prefix << path.path();
}

void WindowContextImpl::addModel(const QString &context,
		const QDBusObjectPath &path) {
	qDebug() << "WindowContextImpl::addModel" << context << path.path();
}
