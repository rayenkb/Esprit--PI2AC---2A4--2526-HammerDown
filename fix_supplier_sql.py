fpath = r'c:\Users\chall\Desktop\4@ (4)\mainwindow.cpp'

with open(fpath, 'rb') as f:
    raw = f.read()

# ──────────────────────────────────────────────────────────────────
# FIX 1: Replace the SELECT query in onSupplierRefreshView
# The reserved-word alias "Delete" can cause Oracle ODBC parse errors.
# Safest approach: just select real columns, no string-literal columns.
# Column order must stay: [0]=Action [1]=Del [2]=ID [3]=Company [4]=Addr [5]=Email [6]=Phone [7]=Type [8]=Postal
# We keep two dummy columns by using numeric literals instead of strings.
# ──────────────────────────────────────────────────────────────────

# Old query block (after emoji fix)
OLD_REFRESH = (
    b'        "SELECT \'Edit\' AS \\"Action\\", \'Delete\' AS \\"Delete\\", SUPPLIER_ID AS \\"ID\\", "\r\n'
    b'        "SUPPLIER_NAME AS \\"Company\\", ADDRESS AS \\"Address\\","\r\n'
    b'        " EMAIL AS \\"Email\\", PHONE_NUMBER AS \\"Phone\\", TYPE_NOTIFICATION AS \\"Type\\","\r\n'
    b'        " POSTAL_CODE AS \\"Postal Code\\""\r\n'
    b'        " FROM SUPPLIERS ORDER BY SUPPLIER_ID"'
)

NEW_REFRESH = (
    b'        "SELECT 1 AS \\"Action\\", 2 AS \\"Del\\", SUPPLIER_ID AS \\"ID\\", "\r\n'
    b'        "SUPPLIER_NAME AS \\"Company\\", ADDRESS AS \\"Address\\","\r\n'
    b'        " EMAIL AS \\"Email\\", PHONE_NUMBER AS \\"Phone\\", TYPE_NOTIFICATION AS \\"Type\\","\r\n'
    b'        " POSTAL_CODE AS \\"Postal Code\\""\r\n'
    b'        " FROM SUPPLIERS ORDER BY SUPPLIER_ID"'
)

# Old search query block
OLD_SEARCH = (
    b'        "SELECT \'Edit\' AS \\"Action\\", \'Delete\' AS \\"Delete\\", SUPPLIER_ID AS \\"ID\\", "\r\n'
    b'        "SUPPLIER_NAME AS \\"Company\\", ADDRESS AS \\"Address\\","\r\n'
    b'        " EMAIL AS \\"Email\\", PHONE_NUMBER AS \\"Phone\\", TYPE_NOTIFICATION AS \\"Type\\","\r\n'
    b'        " POSTAL_CODE AS \\"Postal Code\\""\r\n'
    b'        " FROM SUPPLIERS"'
)

NEW_SEARCH = (
    b'        "SELECT 1 AS \\"Action\\", 2 AS \\"Del\\", SUPPLIER_ID AS \\"ID\\", "\r\n'
    b'        "SUPPLIER_NAME AS \\"Company\\", ADDRESS AS \\"Address\\","\r\n'
    b'        " EMAIL AS \\"Email\\", PHONE_NUMBER AS \\"Phone\\", TYPE_NOTIFICATION AS \\"Type\\","\r\n'
    b'        " POSTAL_CODE AS \\"Postal Code\\""\r\n'
    b'        " FROM SUPPLIERS"'
)

count_r = raw.count(OLD_REFRESH)
count_s = raw.count(OLD_SEARCH)
print(f"Found refresh={count_r}, search={count_s}")

raw = raw.replace(OLD_REFRESH, NEW_REFRESH)
raw = raw.replace(OLD_SEARCH, NEW_SEARCH)

count_r2 = raw.count(OLD_REFRESH)
count_s2 = raw.count(OLD_SEARCH)
count_new = raw.count(b'1 AS \\"Action\\"')
print(f"After: refresh_old={count_r2}, search_old={count_s2}, new_pattern={count_new}")

with open(fpath, 'wb') as f:
    f.write(raw)
print("Done.")
