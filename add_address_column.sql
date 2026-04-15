-- Add address column to EMPLOYEES table with constraint
ALTER TABLE EMPLOYEES ADD (ADDRESS VARCHAR2(200) NOT NULL);

-- Set default values for existing records (optional - remove if you want to manually update)
UPDATE EMPLOYEES SET ADDRESS = 'Not specified' WHERE ADDRESS IS NULL;

EXIT;
