/*
 * Copyright © 2009 Christopher Eby <kreed@kreed.org>
 *
 * This file is part of Tumult.
 *
 * Tumult is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * Tumult is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "hitlist.h"

#include <qdesktopservices.h>
#include <qdir.h>
#include <qtemporaryfile.h>

QString
HitList::location()
{
	QString location = QDesktopServices::storageLocation(QDesktopServices::DataLocation);
	QDir dir(location);
	if (!dir.exists())
		dir.mkpath(dir.absolutePath());
	return location + "/hits";
}

void
HitList::increment(const QString &url)
{
	QTemporaryFile out;
	out.setAutoRemove(false);
	if (!out.open()) {
		qWarning("Unable to open '%s': %s", qPrintable(out.fileName()), qPrintable(out.errorString()));
		return;
	}

	bool written = false;
	QByteArray urlData = (url + '\t').toUtf8();

	QString inLocation = location();
	QFile in(inLocation);

	if (in.exists()) {
		if (!in.open(QIODevice::ReadOnly)) {
			qWarning("Unable to open '%s': %s", qPrintable(in.fileName()), qPrintable(in.errorString()));
			return;
		}

		int urlSize = urlData.size();
		const char *urlBuffer = urlData.data();
		char line[512];
		qint64 size;

		while ((size = in.readLine(line, 512)) != -1) {
			if (!memcmp(line, urlBuffer, urlSize)) {
				if (written)
					continue;

				bool ok = false;
				unsigned count = QString(line + urlSize).toUInt(&ok);
				if (ok) {
					out.write(urlData);
					out.write(QString::number(count + 1).toUtf8());
					out.putChar('\n');
					written = true;
					continue;
				}
			}

			out.write(line, size);
		}
	}

	if (!written) {
		out.write(urlData);
		out.putChar('1');
		out.putChar('\n');
	}

	in.close();
	out.close();

	in.remove();
	out.rename(inLocation);
}
