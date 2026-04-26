#!/usr/bin/env python3
"""
Extracts all order-related MainWindow:: function implementations
from mainwindow.cpp and writes them into order.cpp.
Also removes extracted blocks from mainwindow.cpp.
"""

MAINWINDOW_CPP = r"c:\Users\chall\Desktop\4@ (4)\mainwindow.cpp"
ORDER_CPP      = r"c:\Users\chall\Desktop\4@ (4)\order.cpp"

# Known 1-based start lines of order functions (from grep output)
ORDER_START_LINES = sorted([
    1473, 1537, 1677, 1749, 1787, 1811, 1923, 1962, 2009, 2110,
    2124, 2138, 2148, 2158, 2474, 2983, 3294, 3354, 3414, 4298,
    7815, 8212, 7997, 8030, 8632, 8717,
])

ORDER_CPP_HEADER = """\
#include "order.h"

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ui_order_management.h"
#include "qrcodegen.h"
#include <QShortcut>
#include <QToolTip>
#include <QDateTime>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
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
#include <QSqlQueryModel>
#include <QStandardItemModel>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QTabWidget>
#include <QPixmap>
#include <QComboBox>
#include <QSpinBox>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QProgressBar>
#include <algorithm>

"""


def extract_function_body(lines, start_idx):
    """Brace-match from start_idx, returns (block, end_exclusive)."""
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

    starts_0 = sorted([s - 1 for s in ORDER_START_LINES])

    for s in starts_0:
        print(f"  Line {s+1}: {lines[s].rstrip()}")

    order_blocks = []
    removal_ranges = []

    for start_0 in starts_0:
        block, end_0 = extract_function_body(lines, start_0)
        order_blocks.append(block)
        removal_ranges.append((start_0, end_0))
        print(f"  Extracted lines {start_0+1}–{end_0} ({end_0 - start_0} lines)")

    to_remove = set()
    for (s, e) in removal_ranges:
        for i in range(s, e):
            to_remove.add(i)

    print(f"\nWriting {ORDER_CPP} ...")
    with open(ORDER_CPP, 'w', encoding='utf-8') as f:
        f.write(ORDER_CPP_HEADER)
        for block in order_blocks:
            f.write('\n')
            f.writelines(block)
            f.write('\n')

    print(f"Rewriting {MAINWINDOW_CPP} (removing {len(to_remove)} lines) ...")
    cleaned = [line for idx, line in enumerate(lines) if idx not in to_remove]
    with open(MAINWINDOW_CPP, 'w', encoding='utf-8') as f:
        f.writelines(cleaned)

    print(f"\nDone.")
    print(f"  order.cpp: {len(order_blocks)} functions extracted")
    print(f"  mainwindow.cpp: {total} -> {len(cleaned)} lines")


if __name__ == '__main__':
    main()
