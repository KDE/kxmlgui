/*
 * Copyright (C) 2017  Alexander Potashev <aspotashev@gmail.com>
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "ktoolbarhelper_p.h"

#include <QVector>

#include <klocalizedstring.h>

namespace KToolbarHelper {

QString i18nToolBarName(const QDomElement &element)
{
    QDomElement textElement;
    bool textElementFound = false;
    const QVector<QString> textKeys = {QStringLiteral("text"), QStringLiteral("Text")};
    for (const QString &key : textKeys) {
        QDomNode textNode = element.namedItem(key);
        if (textNode.isElement()) {
            textElement = textNode.toElement();
            textElementFound = true;
            break;
        }
    }

    if (!textElementFound) {
        return element.attribute(QStringLiteral("name"));
    }

    QByteArray domain = textElement.attribute(QStringLiteral("translationDomain")).toUtf8();
    QByteArray text = textElement.text().toUtf8();
    QByteArray context = textElement.attribute(QStringLiteral("context")).toUtf8();

    if (domain.isEmpty()) {
        domain = element.ownerDocument().documentElement().attribute(QStringLiteral("translationDomain")).toUtf8();
        if (domain.isEmpty()) {
            domain = KLocalizedString::applicationDomain();
        }
    }
    QString i18nText;
    if (!text.isEmpty() && !context.isEmpty()) {
        i18nText = i18ndc(domain.constData(), context.constData(), text.constData());
    } else if (!text.isEmpty()) {
        i18nText = i18nd(domain.constData(), text.constData());
    }
    return i18nText;
}

} // namespace KToolbarHelper
