import re

file_path = "c:/Users/chall/Desktop/4@ (4)/4@/mainwindow.ui"

with open(file_path, 'r', encoding='utf-8') as f:
    content = f.read()

# The specific corrupt end line that is repeated
corrupt_line = '      QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top center; padding: 0 5px; background-color: rgb(210, 174, 193); font-weight: bold; color: white; border-radius: 3px; }</string>'

# The correct line (without </string>)
correct_line_content = '      QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top center; padding: 0 5px; background-color: rgb(210, 174, 193); font-weight: bold; color: white; border-radius: 3px; }'

# The QLineEdit style block we attempted to insert
qlineedit_style = """
           QLineEdit {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #FFFFFF, stop:1 #F5F5F5);
            border: 2px solid #8B6F47;
            border-radius: 8px;
            padding: 8px 12px;
            font-size: 14px;
            color: #333;
            selection-background-color: #8B6F47;
            selection-color: white;
        }
        QLineEdit:hover {
            border: 2px solid #A0825A;
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #FFFFFF, stop:1 #FAFAFA);
        }
        QLineEdit:focus {
            border: 2px solid #8B4513; 
            background: #FFFAF0;
            box-shadow: 0 0 8px rgba(139, 69, 19, 0.4);
        }"""

# We look for the pattern where:
# 1. corrupt_line appears
# 2. followed by qlineedit_style (maybe duplicated)
# 3. followed by corrupt_line again

# Actually, simply finding `corrupt_line` + `qlineedit_style` + `qlineedit_style` + `corrupt_line` or similar variations is tricky with regex due to newlines and whitespace.
# But we can iterate and fix.

# Strategy:
# Find occurances of `corrupt_line`
# Check if looking ahead we see `QLineEdit` styles.
# If so, this is a patched block.
# We then rewrite the block.

# Since I already manually fixed one (2530-2571), the script shouldn't break that one.

lines = content.splitlines()
new_lines = []
i = 0
while i < len(lines):
    line = lines[i]
    
    # Check if this is the start of a corrupt block
    # The corrupt block starts with `corrupt_line`
    if corrupt_line.strip() in line.strip():
        # Check if the next few lines are QLineEdit styles
        # If so, we are in the corrupt block start.
        # We need to absorb everything until the NEXT `corrupt_line` inclusive (if it exists)
        # OR just fix the current line if it's the start.
        
        # Actually, the start of the block in my manual fix was:
        # GroupBox title ... </string>
        # QLineEdit ...
        
        # If I want to fix this, I should replace the FIRST `corrupt_line` with `correct_line_content` (no </string>)
        # And check if there is a duplicate `corrupt_line` later and remove it + ensure </string> is at end.
        
        # Let's peek ahead.
        # Check if next lines match qlineedit_style start
        if i+2 < len(lines) and "QLineEdit {" in lines[i+2]:
             # This confirms we are likely in a patched block.
             # Replace current line with correct content (strip </string>)
             new_lines.append(correct_line_content)
             
             # Now skip ahead?
             # We want to skip the DUPLICATE QLineEdit blocks if any?
             # My manual inspection showed TWO copies of QLineEdit styles.
             # 2532-2550 and 2552-2570.
             
             # Let's verify duplication programmatically.
             # We will just Output the QLineEdit style ONCE.
             new_lines.append(qlineedit_style)
             
             # Now we need to consume lines from original file that correspond to the bad duplicates
             # until we hit the terminating corrupt_line.
             
             # Look ahead for the Closing corrupt_line
             found_end = False
             j = i + 1
             while j < len(lines):
                 if corrupt_line.strip() in lines[j].strip():
                     found_end = True
                     # This is the duplicate line at the end.
                     # We should NOT append it.
                     # Instead we assume we are done with the block.
                     # But we need to add </string>.
                     new_lines.append("       </string>")
                     i = j + 1 # Continue matching after this line
                     break
                 j += 1
             
             if found_end:
                 continue
             else:
                 # If we didn't find the end, maybe it wasn't the duplicate pattern?
                 # Should backtrack or handle gracefully.
                 # Fallback: just append the line and continue (no change) if not confident.
                 # identifying "QLineEdit" blocks is safer.
                 pass

    new_lines.append(line)
    i += 1

final_content = "\n".join(new_lines)
with open(file_path, 'w', encoding='utf-8') as f:
    f.write(final_content)

print("Done fixing.")
