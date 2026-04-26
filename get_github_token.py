import tkinter as tk
from tkinter import ttk
import webbrowser
import os

def open_token_page():
    webbrowser.open("https://github.com/settings/tokens/new?description=ProjectC&scopes=repo")

def submit():
    token = entry.get().strip()
    if token:
        out_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), ".gh_token_temp")
        with open(out_path, "w") as f:
            f.write(token)
        root.destroy()
    else:
        lbl_err.config(text="Please enter a token.", fg="red")

root = tk.Tk()
root.title("GitHub Login")
root.geometry("460x200")
root.resizable(False, False)
root.configure(bg="#0d1117")

# Center window
root.update_idletasks()
w = root.winfo_width()
h = root.winfo_height()
x = (root.winfo_screenwidth() // 2) - (w // 2)
y = (root.winfo_screenheight() // 2) - (h // 2)
root.geometry(f"+{x}+{y}")

tk.Label(root, text="GitHub Personal Access Token", font=("Segoe UI", 13, "bold"),
         bg="#0d1117", fg="#f0f6fc").pack(pady=(18, 4))

tk.Label(root, text="Need a token?  Click the link below to generate one (repo scope).",
         font=("Segoe UI", 9), bg="#0d1117", fg="#8b949e").pack()

lnk = tk.Label(root, text="Open github.com/settings/tokens/new", font=("Segoe UI", 9, "underline"),
               bg="#0d1117", fg="#58a6ff", cursor="hand2")
lnk.pack(pady=(0, 10))
lnk.bind("<Button-1>", lambda e: open_token_page())

entry = tk.Entry(root, width=52, show="*", font=("Consolas", 11),
                 bg="#161b22", fg="#f0f6fc", insertbackground="white",
                 relief="flat", bd=6)
entry.pack(ipady=4)
entry.focus()

lbl_err = tk.Label(root, text="", bg="#0d1117", fg="red", font=("Segoe UI", 9))
lbl_err.pack(pady=2)

btn = tk.Button(root, text="Connect", command=submit, font=("Segoe UI", 10, "bold"),
                bg="#238636", fg="white", activebackground="#2ea043",
                relief="flat", padx=20, pady=5, cursor="hand2")
btn.pack(pady=(0, 10))

root.bind("<Return>", lambda e: submit())
root.mainloop()
