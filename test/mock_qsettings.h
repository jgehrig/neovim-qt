#pragma once

#include <QSettings>
#include <QVariant>

namespace NeovimQt { namespace MockQSettings {

/// Overrides QSettings to use the mock format by default
void EnableByDefault() noexcept;

/// Clears all contents store in MockQSettings
void ClearAllContents() noexcept;

/// Overwrites call contents stored in MockQSettings with newValue
void OverwriteContents(QSettings::SettingsMap newValue) noexcept;

/// Reads a value directly from the in-memory mock settings map.
/// Unlike QSettings::value(), this bypasses all file-sync machinery
/// and always reflects the latest value written by any QSettings instance.
QVariant GetValue(const QString& key) noexcept;

}} // namespace NeovimQt::MockQSettings
