-- Clean up old test data
DELETE FROM SUPPLIERS WHERE SUPPLIER_ID IN (100, 200, 300);
DELETE FROM EQUIPMENT WHERE EQUIPMENT_ID IN (50, 60);

-- Insert Suppliers
INSERT INTO SUPPLIERS (SUPPLIER_ID, SUPPLIER_NAME, ADDRESS, EMAIL, PHONE_NUMBER, TYPE_NOTIFICATION, POSTAL_CODE, REGISTRATION_DATE, ACCOUNT_STATUS, OPENING_TIME, CLOSING_TIME, AVERAGE_RATING)
VALUES (100, 'Lumber Mill A', '123 Forest Ave', 'contact@mill-a.com', '5550100', 'Wood', 1001, SYSDATE, 'Active', '08:00', '23:00', 4.5);

INSERT INTO SUPPLIERS (SUPPLIER_ID, SUPPLIER_NAME, ADDRESS, EMAIL, PHONE_NUMBER, TYPE_NOTIFICATION, POSTAL_CODE, REGISTRATION_DATE, ACCOUNT_STATUS, OPENING_TIME, CLOSING_TIME, AVERAGE_RATING)
VALUES (200, 'Steel Works B', '456 Industrial Rd', 'info@steel-b.com', '5550200', 'Metal', 2002, SYSDATE, 'Active', '08:00', '18:00', 4.2);

INSERT INTO SUPPLIERS (SUPPLIER_ID, SUPPLIER_NAME, ADDRESS, EMAIL, PHONE_NUMBER, TYPE_NOTIFICATION, POSTAL_CODE, REGISTRATION_DATE, ACCOUNT_STATUS, OPENING_TIME, CLOSING_TIME, AVERAGE_RATING)
VALUES (300, 'Oak Specialists', '789 Timber Way', 'sales@oakspec.com', '5550300', 'Wood', 3003, SYSDATE, 'Active', '09:00', '21:00', 4.9);

-- Add Notifications for Supplier 100 (Lumber Mill A)
UPDATE SUPPLIERS 
SET NOTIFICATIONS_JSON = '[{"id": 1001, "type": "Strategic Recommendation", "msg": "Inventory low for Oak. Recommend bulk purchase from Lumber Mill A.", "is_read": 0}, {"id": 1002, "type": "Quality Alert", "msg": "Last shipment from Mill A passed 98% quality check.", "is_read": 1}]'
WHERE SUPPLIER_ID = 100;

-- Add Review for Supplier 300
UPDATE SUPPLIERS
SET RATINGS_JSON = '[{"reviewer": "John Carpenter", "content": "Best oak in the region.", "rating": 5, "date": "2024-03-20"}, {"reviewer": "Alice Smith", "content": "Reliable delivery.", "rating": 4.5, "date": "2024-04-10"}]'
WHERE SUPPLIER_ID = 300;

-- Insert Low Stock Equipment (to trigger AI Advisor)
-- Use a subquery to get a valid employee ID
INSERT INTO EQUIPMENT (EQUIPMENT_ID, EQUIPMENT_TYPE, DESCRIPTION, STATUS, PURCHASE_DATE, EMPLOYEE_ID, UNIT_PRICE, QUANTITY, LOCATION, RESPONSABLE)
VALUES (50, 'Wood', 'Oak Planks for fine furniture', 'Good', SYSDATE, (SELECT MIN(EMPLOYEE_ID) FROM EMPLOYEES), 45.5, 2, 'Main Warehouse', 'Warehouse Manager');

INSERT INTO EQUIPMENT (EQUIPMENT_ID, EQUIPMENT_TYPE, DESCRIPTION, STATUS, PURCHASE_DATE, EMPLOYEE_ID, UNIT_PRICE, QUANTITY, LOCATION, RESPONSABLE)
VALUES (60, 'Metal', 'High-strength steel sheets', 'Excellent', SYSDATE, (SELECT MIN(EMPLOYEE_ID) FROM EMPLOYEES), 85.0, 3, 'Annex B', 'Supply Officer');

COMMIT;
EXIT;
