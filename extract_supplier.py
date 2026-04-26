#!/usr/bin/env python3
"""
Extracts all supplier-related MainWindow:: function implementations
from mainwindow.cpp and writes them into supplier.cpp.
Also removes extracted blocks from mainwindow.cpp.
"""

import re

MAINWINDOW_CPP = r"c:\Users\chall\Desktop\4@ (4)\mainwindow.cpp"
SUPPLIER_CPP   = r"c:\Users\chall\Desktop\4@ (4)\supplier.cpp"

# Known 1-based start lines of supplier functions (from grep output)
SUPPLIER_START_LINES = sorted([
    3569, 3588, 3629, 3714, 3800, 3841, 3884, 4040, 4143, 4191,
    4227, 4268, 4299, 4310, 4348, 4368, 4482, 4764, 5575, 9033,
    10596, 10673, 10758, 10803, 10933, 11031, 11085, 11266, 11363,
    11538, 11652, 11699, 11836,
])

SUPPLIER_CPP_HEADER = """\
#include "supplier.h"

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ui_supplier_management.h"
#include "smtpsender.h"
#include <QShortcut>
#include <QToolTip>
#include <QDateTime>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QMovie>
#include <QBuffer>
#include <QScrollBar>
#include <QGraphicsOpacityEffect>
#include <QGraphicsBlurEffect>
#include <QGraphicsDropShadowEffect>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QSequentialAnimationGroup>
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QListWidget>
#include <QScrollArea>
#include <QSlider>
#include <QFileDialog>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QStringListModel>
#include <QCompleter>
#include <QFileInfo>
#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLineEdit>
#include <QMenu>
#include <QPrinter>
#include <QPainter>
#include <QRandomGenerator>
#include <QRegularExpressionValidator>
#include <QSqlQueryModel>
#include <QStandardItemModel>
#include <QtMath>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QTimeEdit>
#include <QTextEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QTableWidget>
#include <QHeaderView>
#include <QPixmap>
#include <QColor>
#include <QPen>
#include <QBrush>
#include <QLinearGradient>
#include <QRadialGradient>
#include <algorithm>

"""


def extract_function_body(lines, start_idx):
    """
    Given 0-based start_idx pointing to the first line of a function definition,
    walks forward matching braces and returns (block_lines, end_idx_exclusive).
    end_idx_exclusive is the first line AFTER the closing brace.
    """
    depth = 0
    found_open = False
    i = start_idx
    while i < len(lines):
        line = lines[i]
        for ch in line:
            if ch == '{':
                depth += 1
                found_open = True
            elif ch == '}':
                depth -= 1
        i += 1
        if found_open and depth == 0:
            break
    return lines[start_idx:i], i


def main():
    print(f"Reading {MAINWINDOW_CPP} ...")
    with open(MAINWINDOW_CPP, 'r', encoding='utf-8', errors='replace') as f:
        lines = f.readlines()

    total = len(lines)
    print(f"Total lines: {total}")

    # Convert 1-based to 0-based indices
    starts_0 = [s - 1 for s in SUPPLIER_START_LINES]

    # Validate
    for s in starts_0:
        print(f"  Line {s+1}: {lines[s].rstrip()}")

    # Extract each block (brace-matched) and record which 0-based line ranges to remove
    supplier_blocks = []
    removal_ranges = []  # list of (start_0, end_0_exclusive)

    for start_0 in starts_0:
        block, end_0 = extract_function_body(lines, start_0)
        supplier_blocks.append(block)
        removal_ranges.append((start_0, end_0))
        print(f"  Extracted lines {start_0+1}–{end_0} ({end_0 - start_0} lines)")

    # Build the set of line indices to remove
    to_remove = set()
    for (s, e) in removal_ranges:
        for i in range(s, e):
            to_remove.add(i)

    # Also remove any blank lines immediately before each supplier function
    # (optional cosmetic cleanup — skip to keep it safe)

    # Write supplier.cpp
    print(f"\nWriting {SUPPLIER_CPP} ...")
    with open(SUPPLIER_CPP, 'w', encoding='utf-8') as f:
        f.write(SUPPLIER_CPP_HEADER)
        for block in supplier_blocks:
            f.write('\n')
            f.writelines(block)
            f.write('\n')

    # Write cleaned mainwindow.cpp
    print(f"Rewriting {MAINWINDOW_CPP} (removing {len(to_remove)} lines) ...")
    cleaned = [line for idx, line in enumerate(lines) if idx not in to_remove]
    with open(MAINWINDOW_CPP, 'w', encoding='utf-8') as f:
        f.writelines(cleaned)

    print(f"\nDone.")
    print(f"  supplier.cpp: {len(supplier_blocks)} functions extracted")
    print(f"  mainwindow.cpp: {total} -> {len(cleaned)} lines")


if __name__ == '__main__':
    main()
