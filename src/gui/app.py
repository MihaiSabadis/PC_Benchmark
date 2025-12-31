import ctypes
import os
import platform
import threading
import time
import customtkinter as ctk
from matplotlib.figure import Figure
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg

REF_SPECS = """Reference System (Grade 10.0):
OS:  Windows 11 Pro
CPU: Intel Core i7-12700H (14 Cores), 20 logical processors
GPU: NVIDIA GeForce RTX 3060 Laptop
RAM: 16.0 GB DDR5"""

# --- CONFIGURATION CONSTANTS ---
TESTS = {
    0: "Integer (MIPS)",
    1: "Float (MFLOPS)",
    2: "Memory Bandwidth (MB/s)",
    3: "AES (MB/s)",
    4: "Compression (MB/s)",
    5: "Memory Latency (MOPS)",
    6: "Disk I/O (MB/s)"
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
        ("comp_bytes", ctypes.c_size_t),
        ("disk_bytes", ctypes.c_size_t)
    ]

# --- LOAD DLL ---
def get_dll_path(dll_name):
    import sys
    system = platform.system()
    if system == "Windows":
        real_name = dll_name
    elif system == "Darwin": # macOS
        real_name = "lib" + dll_name.replace(".dll", ".dylib")
    elif system == "Linux":
        real_name = "lib" + dll_name.replace(".dll", ".so")
    else:
        real_name = dll_name # Fallback

    if getattr(sys, 'frozen', False):
        return os.path.join(sys._MEIPASS, dll_name)
    
    current_dir = os.path.dirname(os.path.abspath(__file__))
    possible_paths = [
        "../../out/build/x64-Release/bin/" + real_name, # Visual Studio
        "../../build/" + real_name,                     # Standard CMake (Mac/Linux)
        "../../bin/" + real_name                        # Generic bin
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
        self.title("SCS PC Benchmark")
        ctk.set_appearance_mode("Dark")

        self.grid_columnconfigure(1, weight=1)
        self.grid_rowconfigure(0, weight=1)

        self.results = {} 
        
        self.setup_sidebar()
        self.setup_main_area()
        
        # Initial Data Load
        self.fetch_system_info()
        self.update_config_display()
        self.change_profile("Standard")

    def setup_sidebar(self):
        self.sidebar = ctk.CTkFrame(self, width=250, corner_radius=0)
        self.sidebar.grid(row=0, column=0, sticky="nsew")
        
        # 1. Header
        self.logo = ctk.CTkLabel(self.sidebar, text="SCS BENCHMARK", font=("Roboto", 22, "bold"))
        self.logo.pack(pady=(30, 10))

        # Grade
        self.lbl_grade = ctk.CTkLabel(self.sidebar, text="Grade: -- / 10", text_color="#00FF00", font=("Arial", 24, "bold"))
        self.lbl_grade.pack(pady=5)

        #Note
        self.lbl_ref_note = ctk.CTkLabel(self.sidebar, 
                                 text="(10.0 = Reference PC)", 
                                 font=("Arial", 10), text_color="gray")
        self.lbl_ref_note.pack(pady=0)

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
        # Create buttons for basic tests
        test_ids = [0, 1, 3, 4, 6] 
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
            f"Compress Data: {to_mb(cfg.comp_bytes)}\n"
            f"Disk Data:     {to_mb(cfg.disk_bytes)}"
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
        # 1. Get User's Info from C
        try:
            buf = ctypes.create_string_buffer(512)
            lib.get_system_info_str(buf, 512)
            user_specs = buf.value.decode("utf-8")
            
            # Update the Sidebar Label (Short version)
            self.lbl_sysinfo.configure(text=user_specs)
            
            # 2. Print Detailed Comparison to Log
            self.log("="*40)
            self.log("      SYSTEM COMPARISON")
            self.log("="*40)
            self.log(f"[YOUR SYSTEM]\n{user_specs}\n")
            self.log("-" * 20)
            self.log(f"[{REF_SPECS}]\n")
            self.log("="*40)
            self.log("Ready to benchmark.\n")
            
        except Exception as e:
            self.log(f"Error fetching system info: {e}")
            self.lbl_sysinfo.configure(text="System Info Error")

    def log(self, msg):
        self.log_box.insert("end", msg + "\n")
        self.log_box.see("end")

    def run_test(self, test_id):
        tname = TESTS.get(test_id, "Unknown")
        self.log(f"--- Starting {tname} ---")
        self.results[test_id] = []

        self.ax.clear()
        self.ax.set_title(f"{tname} Real-Time Evolution", color='#00E5FF', fontsize=12, fontweight='bold')
        self.ax.set_ylabel("Throughput", color='gray')
        self.ax.set_facecolor('#2b2b2b')
        self.ax.grid(True, linestyle=':', alpha=0.3, color='gray')
        
        self.canvas.draw()
        
        threading.Thread(target=self._run_c_logic, args=(test_id,)).start()

    def _run_c_logic(self, test_id):
        def on_progress(run, score):
            self.after(0, self.update_data, test_id, run, score)
        c_cb = CALLBACK_TYPE(on_progress)
        lib.run_test_by_id(test_id, c_cb)

    def update_data(self, test_id, run, score):
        self.results[test_id].append(score)
        
        # X-Axis: 1 to 50
        runs = range(1, len(self.results[test_id])+1)
        
        self.ax.clear()
        
        # --- LINE GRAPH RESTORED ---
        # We use a Cyan line with markers. As points get added, it flows.
        self.ax.plot(runs, self.results[test_id], 
                     color='#00E5FF',      # Cyan Line
                     marker='.',           # Small dots
                     linestyle='-',        # Connected line
                     linewidth=1.5,        # Thin, precise line
                     alpha=0.9)            # Slight transparency

        # Fill under the line for a "Cyberpunk" monitor look
        self.ax.fill_between(runs, self.results[test_id], color='#00E5FF', alpha=0.1)

        # --- TEXT STYLING ---
        tname = TESTS.get(test_id, "")
        self.ax.set_title(f"{tname}", color='#00E5FF', fontsize=12, fontweight='bold')
        self.ax.set_xlabel("Samples (Time)", color='gray', fontsize=9)
        
        # Dynamic Y-Axis scaling so the line doesn't jump too much
        if len(self.results[test_id]) > 1:
            # Zoom in slightly to show variation
            low = min(self.results[test_id]) * 0.95
            high = max(self.results[test_id]) * 1.05
            self.ax.set_ylim(bottom=low, top=high)

        self.ax.grid(axis='both', linestyle=':', alpha=0.3, color='gray')
        self.canvas.draw()
        
        self.log(f"Run {run}: {score:,.1f}")

        # Check against the configured repetitions (which is now 50)
        cfg_ptr = lib.bench_config_defaults()
        max_runs = cfg_ptr.contents.repetitionsK
        
        if run == max_runs: 
             self.calculate_final_grade()

    def calculate_final_grade(self):
        print("--- DEBUG: STARTING GRADE CALCULATION ---")
        total_ratio = 0
        count = 0
        
        # Iterate through all results
        for tid, scores in self.results.items():
            if not scores: 
                print(f"Skipping Test ID {tid} (No scores)")
                continue
                
            best = max(scores)
            ref = lib.get_test_reference(tid)
            
            print(f"Test ID: {tid} | Your Score: {best:.2f} | Reference: {ref:.2f}")
            
            # Check for valid reference AND valid score
            if ref > 0:
                ratio = best / ref
                total_ratio += ratio
                count += 1
                print(f"   -> Added Ratio: {ratio:.4f}")
            else:
                print("   -> IGNORED (Reference is 0)")

        print(f"--- SUMMARY: Count={count}, TotalRatio={total_ratio:.4f} ---")

        if count > 0:
            final_grade = (total_ratio / count) * 10.0
            print(f"✅ CALCULATED GRADE: {final_grade:.2f}")
            
            # FORCE UI UPDATE
            new_text = f"Grade: {final_grade:.2f} / 10"
            self.lbl_grade.configure(text=new_text)
            self.lbl_grade.update() # Force redraw immediately
        else:
            print("❌ NO VALID TESTS FOUND FOR GRADE")

    def run_full_suite(self):
        self.log(">>> FULL SUITE START <<<")
        threading.Thread(target=self._run_full_sequence).start()

    def _run_full_sequence(self):
        # Run standard set (0,1,2,3,4)
        # Note: 2 is Seq Memory. If you want Random, add 5.
        ids_to_run = [0, 1, 2, 3, 4, 5, 6] 
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