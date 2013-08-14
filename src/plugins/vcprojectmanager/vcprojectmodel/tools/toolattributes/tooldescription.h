/**************************************************************************
**
** Copyright (c) 2013 Bojan Petrovic
** Copyright (c) 2013 Radovan Zivkvoic
** Contact: http://www.qt-project.org/legal
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
****************************************************************************/
#ifndef VCPROJECTMANAGER_INTERNAL_TOOLDESCRIPTION_H
#define VCPROJECTMANAGER_INTERNAL_TOOLDESCRIPTION_H

#include <QString>

namespace VcProjectManager {
namespace Internal {

class IAttributeDescriptionDataItem;
class ConfigurationTool;
class ToolSectionDescription;

class ToolDescription
{
public:
    ToolDescription();
    ~ToolDescription();

    int sectionDescriptionCount() const;
    ToolSectionDescription *sectionDescription(int index) const;
    void addSectionDescription(ToolSectionDescription *sectionDescription);
    void removeSectionDescription(ToolSectionDescription *sectionDescription);

    QString toolKey() const;
    void setToolKey(const QString &toolKey);

    QString toolDisplayName() const;
    void setToolDisplayName(const QString &toolDisplayName);

    ConfigurationTool* createTool();

private:
    QString m_displayName;
    QString m_toolKey;
    QList<IAttributeDescriptionDataItem *> m_attributes;
    QList<ToolSectionDescription *> m_sectionDescriptions;
};

} // namespace Internal
} // namespace VcProjectManager

#endif // VCPROJECTMANAGER_INTERNAL_TOOLDESCRIPTION_H