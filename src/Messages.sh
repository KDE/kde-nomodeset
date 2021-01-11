#!/bin/sh
# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2021 Harald Sitter <sitter@kde.org>

$XGETTEXT `find -name \*.cpp -o -name \*.qml -o -name \*.js` -o $podir/kde-nomodeset.pot
