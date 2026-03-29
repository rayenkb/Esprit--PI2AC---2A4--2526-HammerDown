import xml.etree.ElementTree as ET
import re

tree = ET.parse('client_management.ui')
root = tree.getroot()

# Update stylesheets to remove pink and add cyberpunk
for prop in root.iter('property'):
    if prop.get('name') == 'styleSheet':
        for string_elem in prop.iter('string'):
            content = string_elem.text
            if content and 'background-color: rgb(210, 174, 193)' in content:
                content = content.replace('background-color: rgb(210, 174, 193);', "background-color: transparent; border-bottom: 1px solid #8B6F47; font-family: 'Consolas'; color: #D4AF37; font-size: 16px;")
                string_elem.text = content
            
            # HUD inputs
            if content and 'QLineEdit {' in content:
                hud_ql = """QLineEdit {
    background: rgba(0, 0, 0, 0.5);
    border: 1px solid #8B6F47;
    border-left: 3px solid #D4AF37;
    border-radius: 0px;
    padding: 3px 12px;
    font-size: 14px;
    font-weight: bold;
    font-family: 'Consolas';
    color: #F0E0C0;
    selection-background-color: #8B6F47;
    selection-color: white;
}
QLineEdit:hover {
    border: 1px solid #D4AF37;
    border-left: 3px solid #FFD700;
    background: rgba(139, 111, 71, 0.2);
}
QLineEdit:focus {
    border: 1px solid #FFD700;
    border-left: 4px solid #FFD700;
    background: rgba(20, 15, 10, 0.8);
}"""
                content = re.sub(r'QLineEdit\s*\{.*?(?=\s*(?:\n\s*</str|<|$|\w+\s*\{))', hud_ql, content, flags=re.DOTALL)
                string_elem.text = content

# Remove btn_cancel and on_gc_annuler_mod
for parent in root.iter('widget'):
    to_remove = []
    for child in parent.findall('widget'):
        name = child.get('name')
        if name in ['btn_cancel', 'on_gc_annuler_mod']:
            to_remove.append(child)
    for child in to_remove:
        parent.remove(child)

# Add Cyber Trace and Data Matrix tabs tracking
for tab_widget in root.iter('widget'):
    if tab_widget.get('class') == 'QTabWidget' and tab_widget.get('name') == 'tabWidget':
        
        # 1. Cyber Trace tab
        cyber_tab = ET.Element('widget', attrib={'class': 'QWidget', 'name': 'tab_cyber_trace'})
        attr_title = ET.SubElement(cyber_tab, 'attribute', attrib={'name': 'title'})
        ET.SubElement(attr_title, 'string').text = 'Cyber Trace'
        
        tv = ET.SubElement(cyber_tab, 'widget', attrib={'class': 'QTableView', 'name': 'tableView_cyber'})
        prop_geom = ET.SubElement(tv, 'property', attrib={'name': 'geometry'})
        rect = ET.SubElement(prop_geom, 'rect')
        ET.SubElement(rect, 'x').text = '20'
        ET.SubElement(rect, 'y').text = '20'
        ET.SubElement(rect, 'width').text = '1200'
        ET.SubElement(rect, 'height').text = '700'
        
        prop_ss = ET.SubElement(tv, 'property', attrib={'name': 'styleSheet'})
        ET.SubElement(prop_ss, 'string', attrib={'notr': 'true'}).text = "QTableView { background: rgba(0,0,0,0.6); gridline-color: #5A4A32; border: 1px solid #8B6F47; color: #D4AF37; font-family: 'Consolas'; } QHeaderView::section { background: rgba(139,111,71,0.3); border: 1px solid #8B6F47; color: #D4AF37; font-weight: bold; } QTableView::item:selected { background: rgba(139,111,71,0.5); border: 1px solid #D4AF37; }"

        btn_ref = ET.SubElement(cyber_tab, 'widget', attrib={'class': 'QPushButton', 'name': 'btn_refresh_trace'})
        prop_geom_btn = ET.SubElement(btn_ref, 'property', attrib={'name': 'geometry'})
        rect_btn = ET.SubElement(prop_geom_btn, 'rect')
        ET.SubElement(rect_btn, 'x').text = '1070'
        ET.SubElement(rect_btn, 'y').text = '670'
        ET.SubElement(rect_btn, 'width').text = '150'
        ET.SubElement(rect_btn, 'height').text = '40'
        prop_ss_btn = ET.SubElement(btn_ref, 'property', attrib={'name': 'styleSheet'})
        ET.SubElement(prop_ss_btn, 'string', attrib={'notr': 'true'}).text = "QPushButton { background: rgba(139, 111, 71, 0.4); border: 1px solid #8B6F47; border-radius: 5px; color: #D4AF37; font-weight: bold; font-family: 'Consolas'; } QPushButton:hover { background: rgba(139, 111, 71, 0.8); border: 1px solid #D4AF37; }"
        prop_txt_btn = ET.SubElement(btn_ref, 'property', attrib={'name': 'text'})
        ET.SubElement(prop_txt_btn, 'string').text = 'UPDATE LOG'

        tab_widget.append(cyber_tab)
        
        # 2. Data Matrix tab
        matrix_tab = ET.Element('widget', attrib={'class': 'QWidget', 'name': 'tab_data_matrix'})
        attr_title2 = ET.SubElement(matrix_tab, 'attribute', attrib={'name': 'title'})
        ET.SubElement(attr_title2, 'string').text = 'Data Matrix'
        
        frm = ET.SubElement(matrix_tab, 'widget', attrib={'class': 'QFrame', 'name': 'frame_matrix'})
        prop_geom2 = ET.SubElement(frm, 'property', attrib={'name': 'geometry'})
        rect2 = ET.SubElement(prop_geom2, 'rect')
        ET.SubElement(rect2, 'x').text = '100'
        ET.SubElement(rect2, 'y').text = '100'
        ET.SubElement(rect2, 'width').text = '1040'
        ET.SubElement(rect2, 'height').text = '550'
        prop_ss2 = ET.SubElement(frm, 'property', attrib={'name': 'styleSheet'})
        ET.SubElement(prop_ss2, 'string', attrib={'notr': 'true'}).text = "background: rgba(10, 10, 10, 0.7); border: 2px solid #8B6F47; border-radius: 10px;"
        
        lbl = ET.SubElement(matrix_tab, 'widget', attrib={'class': 'QLabel', 'name': 'label_matrix_title'})
        prop_geom3 = ET.SubElement(lbl, 'property', attrib={'name': 'geometry'})
        rect3 = ET.SubElement(prop_geom3, 'rect')
        ET.SubElement(rect3, 'x').text = '370'
        ET.SubElement(rect3, 'y').text = '30'
        ET.SubElement(rect3, 'width').text = '500'
        ET.SubElement(rect3, 'height').text = '40'
        prop_ss3 = ET.SubElement(lbl, 'property', attrib={'name': 'styleSheet'})
        ET.SubElement(prop_ss3, 'string', attrib={'notr': 'true'}).text = "color: #D4AF37; font-size: 24px; font-weight: bold; font-family: 'Consolas';"
        prop_txt = ET.SubElement(lbl, 'property', attrib={'name': 'text'})
        ET.SubElement(prop_txt, 'string').text = '-- DATA MATRIX PROTOCOL ACTIVATED --'
        
        tab_widget.append(matrix_tab)

tree.write('client_management.ui', encoding='UTF-8', xml_declaration=True)
