import ctypes
import os
import threading
import customtkinter as ctk
from matplotlib.figure import Figure
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg

# --- CONFIGURATION ---
TESTS = {
    0: "Integer (MIPS)",
    1: "Float (MFLOPS)",
    2: "Memory (MB/s)",
    3: "AES (MB/s)",
    4: "Compression (MB/s)"
}

# --- LOAD DLL ---
current_dir = os.path.dirname(os.path.abspath(__file__))
# Check both debug/release paths just in case
possible_paths = [
    "../../out/build/x64-Release/bin/pcbench.dll",
    "../../out/build/x64-Debug/bin/pcbench.dll"
]
dll_path = ""
for p in possible_paths:
    full_p = os.path.abspath(os.path.join(current_dir, p))
    if os.path.exists(full_p):
        dll_path = full_p
        break

if not dll_path:
    print("❌ DLL not found! Did you build the project?")
    exit(1)

lib = ctypes.CDLL(dll_path)

# --- DEFINE C SIGNATURES ---
CALLBACK_TYPE = ctypes.CFUNCTYPE(None, ctypes.c_int, ctypes.c_double)
lib.run_test_by_id.argtypes = [ctypes.c_int, CALLBACK_TYPE]
lib.get_test_reference.argtypes = [ctypes.c_int]
lib.get_test_reference.restype = ctypes.c_double
lib.get_system_info_str.argtypes = [ctypes.c_char_p, ctypes.c_int]

class BenchmarkApp(ctk.CTk):
    def __init__(self):
        super().__init__()
        self.geometry("1100x750")
        self.title("SCS PC Benchmark - Team 30431")
        ctk.set_appearance_mode("Dark")

        self.grid_columnconfigure(1, weight=1)
        self.grid_rowconfigure(0, weight=1)

        self.setup_sidebar()
        self.setup_main_area()
        
        self.results = {} 
        
        # Auto-fetch System Info on startup
        self.fetch_system_info()

    def setup_sidebar(self):
        self.sidebar = ctk.CTkFrame(self, width=220, corner_radius=0)
        self.sidebar.grid(row=0, column=0, sticky="nsew")
        
        # Title
        self.logo = ctk.CTkLabel(self.sidebar, text="SCS BENCHMARK", font=("Roboto", 22, "bold"))
        self.logo.pack(pady=(30, 10))

        # System Info Label (Placeholder)
        self.lbl_sysinfo = ctk.CTkLabel(self.sidebar, text="Loading info...", 
                                        font=("Arial", 12), text_color="gray")
        self.lbl_sysinfo.pack(pady=10, padx=10)

        ctk.CTkFrame(self.sidebar, height=2, fg_color="gray40").pack(fill="x", pady=10, padx=20)

        # Buttons
        self.lbl_tests = ctk.CTkLabel(self.sidebar, text="Run Individual Tests:", anchor="w", font=("Arial", 14, "bold"))
        self.lbl_tests.pack(padx=20, pady=(10,5), fill="x")

        self.buttons = {}
        for tid, name in TESTS.items():
            btn = ctk.CTkButton(self.sidebar, text=name.split()[0], 
                                command=lambda i=tid: self.run_test(i))
            btn.pack(padx=20, pady=5)
            self.buttons[tid] = btn

        # Full Suite
        ctk.CTkFrame(self.sidebar, height=2, fg_color="gray40").pack(fill="x", pady=20, padx=20)
        self.btn_full = ctk.CTkButton(self.sidebar, text="RUN FULL SUITE", 
                                      fg_color="green", hover_color="darkgreen",
                                      height=40, font=("Arial", 14, "bold"),
                                      command=self.run_full_suite)
        self.btn_full.pack(padx=20, pady=10)
        
        # Grade
        self.lbl_grade = ctk.CTkLabel(self.sidebar, text="Final Grade:\n-- / 10", font=("Arial", 24, "bold"))
        self.lbl_grade.pack(pady=30, side="bottom")

    def setup_main_area(self):
        self.main_frame = ctk.CTkFrame(self)
        self.main_frame.grid(row=0, column=1, sticky="nsew", padx=20, pady=20)
        
        # Graph Area
        self.fig = Figure(figsize=(5, 4), dpi=100)
        self.fig.patch.set_facecolor('#2b2b2b') # Match dark theme
        self.ax = self.fig.add_subplot(111)
        self.ax.set_facecolor('#2b2b2b')
        self.ax.tick_params(colors='white')
        self.ax.xaxis.label.set_color('white')
        self.ax.yaxis.label.set_color('white')
        self.ax.title.set_color('white')
        
        self.canvas = FigureCanvasTkAgg(self.fig, master=self.main_frame)
        self.canvas.get_tk_widget().pack(fill="both", expand=True, pady=10)

        # Log Area
        self.log_box = ctk.CTkTextbox(self.main_frame, height=150, font=("Consolas", 12))
        self.log_box.pack(fill="x", pady=10)

    def fetch_system_info(self):
        # Create a buffer for C to write into
        buf_size = 512
        buf = ctypes.create_string_buffer(buf_size)
        lib.get_system_info_str(buf, buf_size)
        info_str = buf.value.decode("utf-8")
        self.lbl_sysinfo.configure(text=info_str)

    def log(self, msg):
        self.log_box.insert("end", msg + "\n")
        self.log_box.see("end")

    def run_test(self, test_id):
        self.log(f"--- Starting {TESTS[test_id]} ---")
        self.results[test_id] = []
        
        # Reset graph
        self.ax.clear()
        self.ax.set_title(f"{TESTS[test_id]} Evolution")
        self.ax.set_xlabel("Repetition")
        self.ax.set_ylabel("Throughput")
        self.canvas.draw()
        
        threading.Thread(target=self._run_c_logic, args=(test_id,)).start()

    def _run_c_logic(self, test_id):
        def on_progress(run, score):
            self.after(0, self.update_data, test_id, run, score)
        c_cb = CALLBACK_TYPE(on_progress)
        lib.run_test_by_id(test_id, c_cb)

    def update_data(self, test_id, run, score):
        self.results[test_id].append(score)
        
        # Plot
        runs = range(1, len(self.results[test_id])+1)
        self.ax.clear()
        self.ax.plot(runs, self.results[test_id], marker='o', color='#1f6aa5', linewidth=2)
        self.ax.set_title(f"{TESTS[test_id]}")
        self.ax.grid(True, linestyle='--', alpha=0.3)
        self.canvas.draw()
        
        ref = lib.get_test_reference(test_id)
        # Avoid division by zero
        ratio = (score / ref) if ref > 0 else 0
        self.log(f"Run {run}: {score:,.1f} (Index: {ratio:.2f}x)")

        if run == 5:
            self.calculate_final_grade()

    def calculate_final_grade(self):
        total_ratio = 0
        count = 0
        for tid, scores in self.results.items():
            if not scores: continue
            best_score = max(scores) # Use best score of the 5 runs
            ref = lib.get_test_reference(tid)
            if ref > 0:
                total_ratio += (best_score / ref)
                count += 1
        
        if count > 0:
            # 1.0 index = Grade 10.0
            grade = (total_ratio / count) * 10.0
            self.lbl_grade.configure(text=f"Final Grade:\n{grade:.2f} / 10")

    def run_full_suite(self):
        self.log(">>> STARTING FULL SUITE <<<")
        # In a real app, you'd chain these with callbacks or a queue
        # For simplicity, we launch them one by one in a thread
        threading.Thread(target=self._run_full_sequence).start()

    def _run_full_sequence(self):
        import time
        
        # Iterate through all tests (0 to 4)
        for tid in TESTS:
            # 1. Reset data for this test
            self.results[tid] = []
            
            # 2. Log start
            self.log(f"--- Starting {TESTS[tid]} ---")
            
            # 3. Create a callback specifically for THIS test ID
            # We use _tid=tid to capture the current value of the loop variable
            def on_progress(run, score, _tid=tid):
                self.after(0, self.update_data, _tid, run, score)

            c_cb = CALLBACK_TYPE(on_progress)
            
            # 4. Call C function
            # Since we are in a thread, this BLOCKS here until the C code finishes 5 runs
            lib.run_test_by_id(tid, c_cb)
            
            # 5. Optional: Pause for 1 second so you can see the graph result before it clears
            time.sleep(1.0)
            
        self.log(">>> FULL SUITE COMPLETE <<<")

if __name__ == "__main__":
    app = BenchmarkApp()
    app.mainloop()