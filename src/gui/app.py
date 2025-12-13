import ctypes
import os
import threading
import time
import customtkinter as ctk
from matplotlib.figure import Figure
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg

# --- CONFIGURATION CONSTANTS ---
TESTS = {
    0: "Integer (MIPS)",
    1: "Float (MFLOPS)",
    2: "Memory Bandwidth (MB/s)",
    3: "AES (MB/s)",
    4: "Compression (MB/s)",
    5: "Memory Latency (MOPS)" # New Random Access Test
}

# --- C STRUCTURE MAPPING ---
class BenchConfig(ctypes.Structure):
    _fields_ = [
        ("repetitionsK", ctypes.c_int),
        ("warmup", ctypes.c_int),
        ("integer_block_bytes", ctypes.c_size_t),
        ("integer_passes", ctypes.c_int),
        ("float_N", ctypes.c_size_t),
        ("triad_N", ctypes.c_size_t),
        ("aes_bytes", ctypes.c_size_t),
        ("comp_bytes", ctypes.c_size_t)
    ]

# --- LOAD DLL ---
def get_dll_path(dll_name):
    import sys
    if getattr(sys, 'frozen', False):
        return os.path.join(sys._MEIPASS, dll_name)
    
    current_dir = os.path.dirname(os.path.abspath(__file__))
    possible_paths = [
        "../../out/build/x64-Release/bin/" + dll_name,
        "../../out/build/x64-Debug/bin/" + dll_name,
        "../../bin/" + dll_name
    ]
    for p in possible_paths:
        full_p = os.path.abspath(os.path.join(current_dir, p))
        if os.path.exists(full_p):
            return full_p
    return None

dll_path = get_dll_path("pcbench.dll")
if not dll_path:
    print("❌ DLL not found! Please build the project.")
    exit(1)

lib = ctypes.CDLL(dll_path)

# --- DEFINE C SIGNATURES ---
CALLBACK_TYPE = ctypes.CFUNCTYPE(None, ctypes.c_int, ctypes.c_double)

lib.run_test_by_id.argtypes = [ctypes.c_int, CALLBACK_TYPE]
lib.get_test_reference.argtypes = [ctypes.c_int]
lib.get_test_reference.restype = ctypes.c_double
lib.get_system_info_str.argtypes = [ctypes.c_char_p, ctypes.c_int]

# Config Functions
lib.set_config_profile.argtypes = [ctypes.c_int]
lib.bench_config_defaults.restype = ctypes.POINTER(BenchConfig)

class BenchmarkApp(ctk.CTk):
    def __init__(self):
        super().__init__()
        self.geometry("1200x800")
        self.title("SCS PC Benchmark - Team 30431")
        ctk.set_appearance_mode("Dark")

        self.grid_columnconfigure(1, weight=1)
        self.grid_rowconfigure(0, weight=1)

        self.results = {} 
        
        self.setup_sidebar()
        self.setup_main_area()
        
        # Initial Data Load
        self.fetch_system_info()
        self.update_config_display() # Show default values

    def setup_sidebar(self):
        self.sidebar = ctk.CTkFrame(self, width=250, corner_radius=0)
        self.sidebar.grid(row=0, column=0, sticky="nsew")
        
        # 1. Header
        self.logo = ctk.CTkLabel(self.sidebar, text="SCS BENCHMARK", font=("Roboto", 22, "bold"))
        self.logo.pack(pady=(30, 10))

        self.lbl_sysinfo = ctk.CTkLabel(self.sidebar, text="Loading info...", 
                                        font=("Arial", 12), text_color="gray")
        self.lbl_sysinfo.pack(pady=5, padx=10)

        ctk.CTkFrame(self.sidebar, height=2, fg_color="gray40").pack(fill="x", pady=10, padx=20)

        # 2. PROFILE SELECTOR
        self.lbl_profile = ctk.CTkLabel(self.sidebar, text="Test Profile:", font=("Arial", 14, "bold"))
        self.lbl_profile.pack(padx=20, pady=(5,0), anchor="w")

        self.profile_var = ctk.StringVar(value="Standard")
        self.opt_profile = ctk.CTkOptionMenu(self.sidebar, values=["Quick", "Standard", "Extreme"],
                                             command=self.change_profile, variable=self.profile_var)
        self.opt_profile.pack(padx=20, pady=5)

        # 3. CONFIG DISPLAY BOX
        self.frm_config = ctk.CTkFrame(self.sidebar, fg_color="#333333")
        self.frm_config.pack(padx=20, pady=10, fill="x")
        
        self.lbl_cfg_details = ctk.CTkLabel(self.frm_config, text="Initializing...", 
                                            justify="left", font=("Consolas", 11), text_color="#aaaaaa")
        self.lbl_cfg_details.pack(padx=10, pady=10, anchor="w")

        ctk.CTkFrame(self.sidebar, height=2, fg_color="gray40").pack(fill="x", pady=10, padx=20)

        # 4. TEST BUTTONS
        self.lbl_tests = ctk.CTkLabel(self.sidebar, text="Run Tests:", anchor="w", font=("Arial", 14, "bold"))
        self.lbl_tests.pack(padx=20, pady=(5,5), fill="x")
        
        # Memory Toggle
        self.mem_mode = ctk.StringVar(value="Bandwidth")
        self.switch_mem = ctk.CTkSwitch(self.sidebar, text="Random Access (Latency)", 
                                        command=self.toggle_memory_mode,
                                        variable=self.mem_mode, onvalue="Latency", offvalue="Bandwidth")
        self.switch_mem.pack(padx=20, pady=5)

        self.buttons = {}
        # Create buttons for basic tests (Skip Memory ID 2/5, we handle it specially)
        test_ids = [0, 1, 3, 4] 
        for tid in test_ids:
            btn = ctk.CTkButton(self.sidebar, text=TESTS[tid].split()[0], 
                                command=lambda i=tid: self.run_test(i))
            btn.pack(padx=20, pady=3)
            self.buttons[tid] = btn
            
        # Memory Button (Special)
        self.btn_mem = ctk.CTkButton(self.sidebar, text="Memory (Seq)", command=self.run_memory_test)
        self.btn_mem.pack(padx=20, pady=3)

        ctk.CTkFrame(self.sidebar, height=2, fg_color="gray40").pack(fill="x", pady=20, padx=20)
        
        self.btn_full = ctk.CTkButton(self.sidebar, text="RUN FULL SUITE", 
                                      fg_color="green", hover_color="darkgreen",
                                      height=40, font=("Arial", 14, "bold"),
                                      command=self.run_full_suite)
        self.btn_full.pack(padx=20, pady=10)
        
        self.lbl_grade = ctk.CTkLabel(self.sidebar, text="Grade: -- / 10", font=("Arial", 20, "bold"))
        self.lbl_grade.pack(pady=20, side="bottom")

    def setup_main_area(self):
        self.main_frame = ctk.CTkFrame(self)
        self.main_frame.grid(row=0, column=1, sticky="nsew", padx=20, pady=20)
        
        self.fig = Figure(figsize=(5, 4), dpi=100)
        self.fig.patch.set_facecolor('#2b2b2b')
        self.ax = self.fig.add_subplot(111)
        self.ax.set_facecolor('#2b2b2b')
        self.ax.tick_params(colors='white', labelcolor='white')
        self.ax.title.set_color('white')
        self.ax.xaxis.label.set_color('white')
        self.ax.yaxis.label.set_color('white')
        
        self.canvas = FigureCanvasTkAgg(self.fig, master=self.main_frame)
        self.canvas.get_tk_widget().pack(fill="both", expand=True, pady=10)

        self.log_box = ctk.CTkTextbox(self.main_frame, height=150, font=("Consolas", 12))
        self.log_box.pack(fill="x", pady=10)

    # --- LOGIC ---

    def change_profile(self, selection):
        map_prof = {"Quick": 0, "Standard": 1, "Extreme": 2}
        pid = map_prof.get(selection, 1)
        
        # 1. Update C Core
        lib.set_config_profile(pid)
        self.log(f"Profile changed to: {selection}")
        
        # 2. Update Display
        self.update_config_display()

    def update_config_display(self):
        # Read the Struct from C
        cfg_ptr = lib.bench_config_defaults()
        cfg = cfg_ptr.contents
        
        # Format Bytes to MiB
        to_mb = lambda b: f"{b / (1024*1024):.0f} MiB"
        
        text = (
            f"Runs per Test: {cfg.repetitionsK}\n"
            f"Integer Data:  {to_mb(cfg.integer_block_bytes)}\n"
            f"Float Count:   {cfg.float_N:,} elems\n"
            f"Memory Array:  {cfg.triad_N:,} elems\n"
            f"AES Data:      {to_mb(cfg.aes_bytes)}\n"
            f"Compress Data: {to_mb(cfg.comp_bytes)}"
        )
        self.lbl_cfg_details.configure(text=text)

    def toggle_memory_mode(self):
        mode = self.mem_mode.get()
        if mode == "Latency":
            self.btn_mem.configure(text="Memory (Rnd)")
        else:
            self.btn_mem.configure(text="Memory (Seq)")

    def run_memory_test(self):
        # Decide which ID to run based on toggle
        tid = 5 if self.mem_mode.get() == "Latency" else 2
        self.run_test(tid)

    def fetch_system_info(self):
        try:
            buf = ctypes.create_string_buffer(512)
            lib.get_system_info_str(buf, 512)
            self.lbl_sysinfo.configure(text=buf.value.decode("utf-8"))
        except: pass

    def log(self, msg):
        self.log_box.insert("end", msg + "\n")
        self.log_box.see("end")

    def run_test(self, test_id):
        tname = TESTS.get(test_id, "Unknown")
        self.log(f"--- Starting {tname} ---")
        self.results[test_id] = []
        
        self.ax.clear()
        self.ax.set_title(f"{tname} Evolution")
        self.ax.set_ylabel("Throughput / Score")
        self.canvas.draw()
        
        threading.Thread(target=self._run_c_logic, args=(test_id,)).start()

    def _run_c_logic(self, test_id):
        def on_progress(run, score):
            self.after(0, self.update_data, test_id, run, score)
        c_cb = CALLBACK_TYPE(on_progress)
        lib.run_test_by_id(test_id, c_cb)

    def update_data(self, test_id, run, score):
        self.results[test_id].append(score)
        
        runs = range(1, len(self.results[test_id])+1)
        self.ax.clear()
        self.ax.plot(runs, self.results[test_id], marker='o', color='#1f6aa5')
        self.ax.set_title(TESTS.get(test_id, ""))
        self.ax.grid(True, alpha=0.3)
        self.canvas.draw()
        
        self.log(f"Run {run}: {score:,.1f}")

        if run == 5: 
             self.calculate_final_grade()

    def calculate_final_grade(self):
        total_ratio = 0
        count = 0
        for tid, scores in self.results.items():
            if not scores: continue
            best = max(scores)
            ref = lib.get_test_reference(tid)
            if ref > 0:
                total_ratio += (best / ref)
                count += 1
        if count > 0:
            self.lbl_grade.configure(text=f"Grade: {(total_ratio/count)*10.0:.2f} / 10")

    def run_full_suite(self):
        self.log(">>> FULL SUITE START <<<")
        threading.Thread(target=self._run_full_sequence).start()

    def _run_full_sequence(self):
        # Run standard set (0,1,2,3,4)
        # Note: 2 is Seq Memory. If you want Random, add 5.
        ids_to_run = [0, 1, 2, 3, 4] 
        for tid in ids_to_run:
            self.after(0, self.log, f"Running {TESTS[tid]}...")
            self.results[tid] = []
            
            def on_progress(run, score, _tid=tid):
                self.after(0, self.update_data, _tid, run, score)
            
            c_cb = CALLBACK_TYPE(on_progress)
            lib.run_test_by_id(tid, c_cb)
            time.sleep(0.5)
        self.after(0, self.log, ">>> DONE <<<")

if __name__ == "__main__":
    app = BenchmarkApp()
    app.mainloop()