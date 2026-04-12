-- Cleanup previous failed insert if any
DELETE FROM EQUIPMENT WHERE EQUIPMENT_ID = 3001;

-- Add low stock wood with correct status and extra columns
INSERT INTO EQUIPMENT (EQUIPMENT_ID, EQUIPMENT_TYPE, DESCRIPTION, STATUS, PURCHASE_DATE, EMPLOYEE_ID, UNIT_PRICE, QUANTITY, LOCATION, RESPONSABLE)
VALUES (3001, 'Oak Wood Planks', 'Premium raw oak for testing AI', 'Available', SYSDATE, 1, 45.0, 2, 'Workshop Main', 'Amine');

-- Link 'Tech Supplies Tunis' (9991) to this material category
-- This will give AI a high-quality historical reference
UPDATE SUPPLIERS 
SET RATINGS_JSON = '[{"rating":5, "note":"Elite oak quality, best for planks.", "employee_id":1, "equipment_id":3001, "date":"2024-03-24T12:00:00"}]',
    AVERAGE_RATING = 5.0
WHERE SUPPLIER_ID = 9991;

-- Link 'Marsa Hardware' (9992) with a mediocre rating for balance
UPDATE SUPPLIERS 
SET RATINGS_JSON = '[{"rating":3, "note":"Good but overpriced.", "employee_id":1, "equipment_id":3001, "date":"2024-03-22T10:00:00"}]',
    AVERAGE_RATING = 4.0
WHERE SUPPLIER_ID = 9992;

COMMIT;
EXIT;
