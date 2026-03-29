import xml.etree.ElementTree as ET

file_path = "c:/Users/chall/Desktop/4@ (4)/4@/mainwindow.ui"

try:
    tree = ET.parse(file_path)
    root = tree.getroot()
    print("XML Parsed successfully.")
    
    # Check for duplicate properties in widgets
    for widget in root.iter("widget"):
        properties = []
        for prop in widget.findall("property"):
            name = prop.get("name")
            if name in properties:
                print(f"Warning: Duplicate property '{name}' in widget '{widget.get('name')}'")
            properties.append(name)

except ET.ParseError as e:
    print(f"XML Parse Error: {e}")
except Exception as e:
    print(f"Error: {e}")
