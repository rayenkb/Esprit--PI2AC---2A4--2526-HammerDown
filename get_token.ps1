Add-Type -AssemblyName System.Windows.Forms
Add-Type -AssemblyName System.Drawing

$form = New-Object System.Windows.Forms.Form
$form.Text = "GitHub Login"
$form.Size = New-Object System.Drawing.Size(460, 230)
$form.StartPosition = "CenterScreen"
$form.BackColor = [System.Drawing.Color]::FromArgb(13, 17, 23)
$form.FormBorderStyle = "FixedDialog"
$form.MaximizeBox = $false
$form.TopMost = $true

$title = New-Object System.Windows.Forms.Label
$title.Text = "GitHub Personal Access Token"
$title.Font = New-Object System.Drawing.Font("Segoe UI", 13, [System.Drawing.FontStyle]::Bold)
$title.ForeColor = [System.Drawing.Color]::FromArgb(240, 246, 252)
$title.BackColor = [System.Drawing.Color]::FromArgb(13, 17, 23)
$title.AutoSize = $true
$title.Location = New-Object System.Drawing.Point(70, 18)
$form.Controls.Add($title)

$sub = New-Object System.Windows.Forms.Label
$sub.Text = "Paste your token below  (github.com/settings/tokens → repo scope)"
$sub.Font = New-Object System.Drawing.Font("Segoe UI", 9)
$sub.ForeColor = [System.Drawing.Color]::FromArgb(139, 148, 158)
$sub.BackColor = [System.Drawing.Color]::FromArgb(13, 17, 23)
$sub.AutoSize = $true
$sub.Location = New-Object System.Drawing.Point(20, 55)
$form.Controls.Add($sub)

$textBox = New-Object System.Windows.Forms.TextBox
$textBox.Location = New-Object System.Drawing.Point(20, 85)
$textBox.Size = New-Object System.Drawing.Size(410, 28)
$textBox.Font = New-Object System.Drawing.Font("Consolas", 11)
$textBox.BackColor = [System.Drawing.Color]::FromArgb(22, 27, 34)
$textBox.ForeColor = [System.Drawing.Color]::FromArgb(240, 246, 252)
$textBox.BorderStyle = "FixedSingle"
$textBox.UseSystemPasswordChar = $true
$form.Controls.Add($textBox)

$btn = New-Object System.Windows.Forms.Button
$btn.Text = "Connect"
$btn.Location = New-Object System.Drawing.Point(170, 130)
$btn.Size = New-Object System.Drawing.Size(110, 35)
$btn.Font = New-Object System.Drawing.Font("Segoe UI", 10, [System.Drawing.FontStyle]::Bold)
$btn.BackColor = [System.Drawing.Color]::FromArgb(35, 134, 54)
$btn.ForeColor = [System.Drawing.Color]::White
$btn.FlatStyle = "Flat"
$btn.FlatAppearance.BorderSize = 0
$btn.DialogResult = [System.Windows.Forms.DialogResult]::OK
$form.AcceptButton = $btn
$form.Controls.Add($btn)

$form.Add_Shown({ $textBox.Select() })
$result = $form.ShowDialog()

if ($result -eq [System.Windows.Forms.DialogResult]::OK) {
    $token = $textBox.Text.Trim()
    if ($token -ne "") {
        $outPath = Join-Path $PSScriptRoot ".gh_token_temp"
        Set-Content -Path $outPath -Value $token -NoNewline
    }
}
