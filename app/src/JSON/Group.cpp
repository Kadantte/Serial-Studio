/*
 * Serial Studio - https://serial-studio.github.io/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR Commercial
 */

#include <QJsonArray>
#include "JSON/Group.h"

/**
 * @brief Reads a value from a QJsonObject based on a key, returning a default
 *        value if the key does not exist.
 *
 * This function checks if the given key exists in the provided QJsonObject.
 * If the key is found, it returns the associated value. Otherwise, it returns
 * the specified default value.
 *
 * @param object The QJsonObject to read the data from.
 * @param key The key to look for in the QJsonObject.
 * @param defaultValue The value to return if the key is not found in the
 * QJsonObject.
 * @return The value associated with the key, or the defaultValue if the key is
 * not present.
 */
static QVariant SAFE_READ(const QJsonObject &object, const QString &key,
                          const QVariant &defaultValue)
{
  if (object.contains(key))
    return object.value(key);

  return defaultValue;
}

/**
 * @brief Constructor function
 */
JSON::Group::Group(const int groupId)
  : m_groupId(groupId)
  , m_title("")
  , m_widget("")
{
}

/**
 * Destructor function
 */
JSON::Group::~Group()
{
  m_datasets.clear();
  m_datasets.squeeze();
}

/**
 * @brief Serializes the group information and its associated datasets into a
 * QJsonObject.
 *
 * This function encodes the group's properties (title and widget) and each
 * dataset within the group into a JSON object. Calls the `encode()` method for
 * each dataset to ensure that all dataset details are properly serialized.
 *
 * @return A QJsonObject containing the group's properties and an array of
 * encoded datasets.
 */
QJsonObject JSON::Group::serialize() const
{
  QJsonArray datasetArray;
  for (const auto &dataset : m_datasets)
    datasetArray.append(dataset.serialize());

  QJsonObject object;
  object.insert(QStringLiteral("title"), m_title.simplified());
  object.insert(QStringLiteral("widget"), m_widget.simplified());
  object.insert(QStringLiteral("datasets"), datasetArray);
  return object;
}

/**
 * Reads the group information and all its asociated datasets from the given
 * JSON @c object.
 *
 * @return @c true on success, @c false on failure
 */
bool JSON::Group::read(const QJsonObject &object)
{
  if (!object.isEmpty())
  {
    // clang-format off
    const auto array = object.value(QStringLiteral("datasets")).toArray();
    const auto title = SAFE_READ(object, "title", "").toString().simplified();
    const auto widget = SAFE_READ(object, "widget", "").toString().simplified();
    // clang-format on

    if (!title.isEmpty() && !array.isEmpty())
    {
      m_title = title;
      m_widget = widget;
      m_datasets.clear();
      m_datasets.squeeze();

      for (auto i = 0; i < array.count(); ++i)
      {
        const auto object = array.at(i).toObject();
        if (!object.isEmpty())
        {
          Dataset dataset(m_groupId, i);
          if (dataset.read(object))
            m_datasets.append(dataset);
        }
      }

      return datasetCount() > 0;
    }
  }

  return false;
}

/**
 * @return The title/description of this group
 */
const QString &JSON::Group::title() const
{
  return m_title;
}

/**
 * @return The widget type of this group (if any)
 */
const QString &JSON::Group::widget() const
{
  return m_widget;
}

/**
 * @return The group groupId in the project array, only used for interacting
 *         with the project model (which is used to build the Project Editor
 *         GUI).
 */
int JSON::Group::groupId() const
{
  return m_groupId;
}

/**
 * @return The number of datasets inside this group
 */
int JSON::Group::datasetCount() const
{
  return m_datasets.count();
}

/**
 * @return A list with all the dataset objects contained in this group
 */
const QVector<JSON::Dataset> &JSON::Group::datasets() const
{
  return m_datasets;
}

/**
 * @return The dataset at the given @a index,vreturns @c nullptr on invalid
 * index
 */
const JSON::Dataset &JSON::Group::getDataset(const int index) const
{
  return m_datasets.at(index);
}
