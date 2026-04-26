@echo off
cd /d "D:\Exprix\Projet C v2"
set LOG="D:\Exprix\Projet C v2\git_output.txt"

echo === Git version === > %LOG%
git --version >> %LOG% 2>&1

echo === Safe dir === >> %LOG%
git config --global --add safe.directory "D:/Exprix/Projet C v2" >> %LOG% 2>&1

echo === Status === >> %LOG%
git status >> %LOG% 2>&1

echo === Add === >> %LOG%
git -c user.name="GambaGs" -c user.email="gaalichem3@gmail.com" add -A >> %LOG% 2>&1
echo Add exit: %errorlevel% >> %LOG%

echo === Commit === >> %LOG%
git -c user.name="GambaGs" -c user.email="gaalichem3@gmail.com" commit -m "WIP: save before supplier-branch merge" >> %LOG% 2>&1
echo Commit exit: %errorlevel% >> %LOG%

echo === Fetch === >> %LOG%
git -c user.name="GambaGs" -c user.email="gaalichem3@gmail.com" fetch https://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@github.com/SkrrtTn/projectc-.git supplier-branch >> %LOG% 2>&1
echo Fetch exit: %errorlevel% >> %LOG%

echo === Merge === >> %LOG%
git -c user.name="GambaGs" -c user.email="gaalichem3@gmail.com" merge FETCH_HEAD --no-edit >> %LOG% 2>&1
echo Merge exit: %errorlevel% >> %LOG%

echo === Log === >> %LOG%
git log --oneline -5 >> %LOG% 2>&1

echo Done. Check git_output.txt
