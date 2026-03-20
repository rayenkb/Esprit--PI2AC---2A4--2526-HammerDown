void MainWindow::setupGlobalStyles()
{
    QString style = R"(
        /* --- General Application Style --- */
        QWidget {
            font-family: 'Gadugi', 'Segoe UI', sans-serif;
            font-size: 14px;
        }

        /* --- Buttons --- */
        QPushButton {
            background-color: #8B6F47; /* Gold/Brown */
            color: white;
            border-radius: 5px;
            padding: 8px 15px;
            font-weight: bold;
            border: 1px solid #6d5638;
        }
        QPushButton:hover {
            background-color: #a38253;
            border: 1px solid #8B6F47;
        }
        QPushButton:pressed {
            background-color: #6d5638;
        }
        QPushButton:disabled {
            background-color: #cccccc;
            color: #666666;
            border: 1px solid #aaaaaa;
        }

        /* --- Input Fields --- */
        QLineEdit, QTextEdit, QPlainTextEdit, QSpinBox, QDoubleSpinBox, QDateEdit, QComboBox {
            background-color: white;
            border: 1px solid #cccccc;
            border-radius: 4px;
            padding: 5px;
            color: #333333;
            selection-background-color: #8B6F47;
            selection-color: white;
        }
        QLineEdit:focus, QTextEdit:focus, QPlainTextEdit:focus, QSpinBox:focus, QDoubleSpinBox:focus, QDateEdit:focus, QComboBox:focus {
            border: 1px solid #8B6F47;
        }

        /* --- Group Boxes --- */
        QGroupBox {
            border: 1px solid #8B6F47;
            border-radius: 6px;
            margin-top: 24px; /* Leave space for title */
            background-color: rgba(255, 255, 255, 0.8); /* Slight transparency */
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            subcontrol-position: top center;
            padding: 5px 10px;
            background-color: #8B6F47;
            color: white;
            border-radius: 4px;
            font-weight: bold;
        }

        /* --- Tab Widget --- */
        QTabWidget::pane {
            border: 1px solid #cccccc;
            background: rgba(255, 255, 255, 0.9);
            border-radius: 4px;
        }
        QTabWidget::tab-bar {
            left: 5px; /* move to the right by 5px */
        }
        QTabBar::tab {
            background: #e0e0e0;
            border: 1px solid #cccccc;
            border-bottom-color: #cccccc; /* same as the pane color */
            border-top-left-radius: 4px;
            border-top-right-radius: 4px;
            min-width: 8ex;
            padding: 8px 15px;
            margin-right: 2px;
            color: #333;
        }
        QTabBar::tab:selected, QTabBar::tab:hover {
            background: #8B6F47;
            color: white;
            border-color: #8B6F47;
        }

        /* --- Tables & Lists --- */
        QTableView, QListWidget {
            border: 1px solid #cccccc;
            gridline-color: #eeeeee;
            background-color: white;
            selection-background-color: rgba(139, 111, 71, 0.2); /* Light Gold */
            selection-color: black;
            alternate-background-color: #f9f9f9;
        }
        QHeaderView::section {
            background-color: #8B6F47;
            color: white;
            padding: 5px;
            border: none;
            font-weight: bold;
        }
        
        /* --- Scrollbars --- */
        QScrollBar:vertical {
            border: none;
            background: #f0f0f0;
            width: 10px;
            margin: 0px 0px 0px 0px;
        }
        QScrollBar::handle:vertical {
            background: #cdcdcd;
            min-height: 20px;
            border-radius: 5px;
        }
        QScrollBar::handle:vertical:hover {
            background: #8B6F47;
        }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
            height: 0px;
        }
    )";
    
    // Apply style to the entire application to ensure consistency
    qApp->setStyleSheet(style);
}
