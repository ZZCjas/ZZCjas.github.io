import tkinter as tk
from tkinter import ttk, messagebox
import numpy as np
import math

G = 1.0

class NBodySim:
    def __init__(self):
        self.pos  = np.zeros((0, 2), dtype=float)
        self.vel  = np.zeros((0, 2), dtype=float)
        self.mass = np.zeros((0, 1), dtype=float)
        self.diam = np.zeros((0, 1), dtype=float)
        # 是否为碎片的标志（True 表示碎片，飞出边界时会被删除）
        self.is_frag = np.zeros((0, 1), dtype=bool)
        self.t = 0.0
        # 存储加速度信息用于计算Δv
        self.acc = np.zeros((0, 2), dtype=float)
        # 存储上一帧的速度用于计算Δv
        self.prev_vel = np.zeros((0, 2), dtype=float)

    @property
    def n(self):
        return self.pos.shape[0]

    def add_body(self, x, y, vx, vy, m, d, is_fragment=False):
        self.pos  = np.vstack([self.pos,  [[x, y]]])
        self.vel  = np.vstack([self.vel,  [[vx, vy]]])
        self.mass = np.vstack([self.mass, [[m]]])
        self.diam = np.vstack([self.diam, [[d]]])
        self.is_frag = np.vstack([self.is_frag, [[bool(is_fragment)]]])
        self.acc = np.vstack([self.acc, [[0.0, 0.0]]])
        self.prev_vel = np.vstack([self.prev_vel, [[vx, vy]]])

    def clear(self):
        self.__init__()

    def accelerations(self):
        n = self.n
        if n == 0:
            return np.zeros((0, 2))
        r_i = self.pos[:, None, :]
        r_j = self.pos[None, :, :]
        diff = r_j - r_i
        # 为防止自作用力造成 div0，使用对角上一个非常小的数而非 1（更合理）
        eps = 1e-12
        r2 = np.sum(diff**2, axis=2) + np.eye(n) * eps
        r = np.sqrt(r2)
        inv_r3 = 1.0 / (r2 * r + 1e-30)
        m_j = self.mass[None, :, :]
        a = G * np.sum(diff * inv_r3[:, :, None] * m_j, axis=1)
        return a

    def detect_collision(self):
        n = self.n
        if n < 2:
            return False, None
        r_i = self.pos[:, None, :]
        r_j = self.pos[None, :, :]
        diff = r_j - r_i
        dist = np.sqrt(np.sum(diff**2, axis=2))
        iu = np.triu_indices(n, k=1)
        sum_r = (self.diam/2.0 + self.diam.T/2.0)
        collided = dist[iu] <= sum_r[iu]
        if np.any(collided):
            k = np.where(collided)[0][0]
            i, j = iu[0][k], iu[1][k]
            return True, (i, j, dist[i, j])
        return False, None

    def step_leapfrog(self, dt, box_L, reflect=True):
        if self.n == 0:
            return
        
        # 保存当前速度用于计算Δv
        self.prev_vel = self.vel.copy()
        
        a_now = self.accelerations()
        self.vel += 0.5 * dt * a_now
        self.pos += dt * self.vel

        # 边界反射仅当 reflect=True 时对非碎片体生效；如果 reflect=False 则不反射
        if reflect:
            for ax in (0, 1):
                # 只对非碎片做反射
                nonfrag_idx = (~self.is_frag[:,0])
                if np.any(nonfrag_idx):
                    hi = (self.pos[nonfrag_idx, ax] > box_L)
                    lo = (self.pos[nonfrag_idx, ax] < -box_L)
                    # 将索引映射回总体索引
                    idxs = np.nonzero(nonfrag_idx)[0]
                    if np.any(hi):
                        sel = idxs[hi]
                        self.pos[sel, ax] = box_L - (self.pos[sel, ax] - box_L)
                        self.vel[sel, ax] *= -1
                    if np.any(lo):
                        sel = idxs[lo]
                        self.pos[sel, ax] = -box_L - (self.pos[sel, ax] + box_L)
                        self.vel[sel, ax] *= -1

        a_new = self.accelerations()
        self.vel += 0.5 * dt * a_new
        
        # 保存当前加速度（取中值近似）
        self.acc = (a_now + a_new) / 2
        
        self.t += dt

    # 安全删除指定索引（接受可迭代索引，自动降序删除）
    def delete_indices(self, idxs):
        idxs_sorted = sorted(set(int(i) for i in idxs if 0 <= int(i) < self.n), reverse=True)
        for idx in idxs_sorted:
            self.pos = np.delete(self.pos, idx, axis=0)
            self.vel = np.delete(self.vel, idx, axis=0)
            self.mass = np.delete(self.mass, idx, axis=0)
            self.diam = np.delete(self.diam, idx, axis=0)
            self.is_frag = np.delete(self.is_frag, idx, axis=0)
            self.acc = np.delete(self.acc, idx, axis=0)
            self.prev_vel = np.delete(self.prev_vel, idx, axis=0)

class NBodyApp:
    def __init__(self, root):
        self.root = root
        root.title("天体运动模拟v1 by ZZCjas")

        self.sim = NBodySim()
        self.canvas_size = 720
        self.box_L = tk.DoubleVar(value=50.0)
        self.dt = tk.DoubleVar(value=0.02)
        self.running = False

        self.new_mass = tk.DoubleVar(value=10.0)
        self.new_diam = tk.DoubleVar(value=2.0)
        self.init_vx  = tk.DoubleVar(value=0.0)
        self.init_vy  = tk.DoubleVar(value=0.0)

        # 可调整参数（你可以改动这些以进一步控制碎片行为）
        self.roche_k = tk.DoubleVar(value=2.3)      # 洛希极限系数
        self.max_total_frags = tk.IntVar(value=20)    # 全局碎片上限
        self.similar_thresh = 0.4      # 近质量碎裂阈值
        self.min_frag_mass   = 0.05    # 碎片最小质量（增大以减少数量）
        self.small_frag_scale_limit = 4  # 小体粉碎时最多碎片数（缩小）

        self.rng = np.random.default_rng()
        self._build_ui()
        self._draw()
        self._schedule_loop()

    def _build_ui(self):
        main = ttk.Frame(self.root)
        main.pack(fill="both", expand=True)

        # 画布
        self.canvas = tk.Canvas(main, width=self.canvas_size, height=self.canvas_size, bg="black", highlightthickness=0)
        self.canvas.grid(row=0, column=0, rowspan=30, padx=8, pady=8)
        self.canvas.bind("<Button-1>", self.on_canvas_click)

        # 右侧面板 + 表格
        panel = ttk.Frame(main)
        panel.grid(row=0, column=1, sticky="n", padx=8, pady=8)

        r = 0
        ttk.Label(panel, text="模拟控制", font=("Microsoft YaHei", 11, "bold")).grid(row=r, column=0, sticky="w"); r += 1
        row1 = ttk.Frame(panel); row1.grid(row=r, column=0, sticky="w", pady=(2,6)); r += 1
        ttk.Button(row1, text="开始/暂停", command=self.toggle_run).grid(row=0, column=0, padx=(0,6))
        ttk.Button(row1, text="单步", command=self.single_step).grid(row=0, column=1, padx=(0,6))
        ttk.Button(row1, text="重置", command=self.reset).grid(row=0, column=2)

        ttk.Label(panel, text="时间步长 dt").grid(row=r, column=0, sticky="w"); r += 1
        ttk.Scale(panel, from_=0.001, to=0.2, variable=self.dt, orient="horizontal", length=220).grid(row=r, column=0, sticky="w")
        ttk.Entry(panel, textvariable=self.dt, width=10).grid(row=r, column=0, sticky="e"); r += 1

        ttk.Label(panel, text="边界半边长 L").grid(row=r, column=0, sticky="w"); r += 1
        ttk.Scale(panel, from_=10, to=200, variable=self.box_L, orient="horizontal", length=220).grid(row=r, column=0, sticky="w")
        ttk.Entry(panel, textvariable=self.box_L, width=10).grid(row=r, column=0, sticky="e"); r += 1

        ttk.Label(panel, text="洛希极限系数").grid(row=r, column=0, sticky="w"); r += 1
        ttk.Scale(panel, from_=1.0, to=5.0, variable=self.roche_k, orient="horizontal", length=220).grid(row=r, column=0, sticky="w")
        ttk.Entry(panel, textvariable=self.roche_k, width=10).grid(row=r, column=0, sticky="e"); r += 1

        ttk.Label(panel, text="最大碎片数量").grid(row=r, column=0, sticky="w"); r += 1
        ttk.Scale(panel, from_=2, to=20, variable=self.max_total_frags, orient="horizontal", length=220).grid(row=r, column=0, sticky="w")
        ttk.Entry(panel, textvariable=self.max_total_frags, width=10).grid(row=r, column=0, sticky="e"); r += 1

        ttk.Separator(panel, orient='horizontal').grid(row=r, column=0, sticky="ew", pady=8); r += 1

        ttk.Label(panel, text="添加天体（点击画布放置）", font=("Microsoft YaHei", 11, "bold")).grid(row=r, column=0, sticky="w"); r += 1
        ttk.Label(panel, text="质量 m").grid(row=r, column=0, sticky="w"); r += 1
        ttk.Scale(panel, from_=1, to=200, variable=self.new_mass, orient="horizontal", length=220).grid(row=r, column=0, sticky="w")
        ttk.Entry(panel, textvariable=self.new_mass, width=10).grid(row=r, column=0, sticky="e"); r += 1
        ttk.Label(panel, text="直径 d").grid(row=r, column=0, sticky="w"); r += 1
        ttk.Scale(panel, from_=0.5, to=10, variable=self.new_diam, orient="horizontal", length=220).grid(row=r, column=0, sticky="w")
        ttk.Entry(panel, textvariable=self.new_diam, width=10).grid(row=r, column=0, sticky="e"); r += 1

        vframe = ttk.Frame(panel); vframe.grid(row=r, column=0, sticky="w", pady=(6,0)); r += 1
        ttk.Label(vframe, text="初速度 vx, vy").grid(row=0, column=0, sticky="w")
        ttk.Entry(vframe, textvariable=self.init_vx, width=8).grid(row=0, column=1, padx=4)
        ttk.Entry(vframe, textvariable=self.init_vy, width=8).grid(row=0, column=2, padx=4)

        toolrow = ttk.Frame(panel); toolrow.grid(row=r, column=0, sticky="w", pady=(8,0)); r += 1
        ttk.Button(toolrow, text="清空所有天体", command=self.clear_bodies).grid(row=0, column=0, padx=(0,8))
        ttk.Button(toolrow, text="示例：双星系统", command=self.load_example_binary).grid(row=0, column=1, padx=(0,8))
        ttk.Button(toolrow, text="示例：三体运动", command=self.load_example_three).grid(row=0, column=2, padx=(0,8))
        # 新增引力弹弓示例按钮
        ttk.Button(toolrow, text="示例：引力弹弓", command=self.load_example_slingshot).grid(row=0, column=3)

        # 统计信息标签
        self.stats_text = tk.StringVar(value="天体: 0, 碎片: 0 | 最大速度: 0.00, 最大加速度: 0.00, 最大质量: 0.00 | 碎片最大速度: 0.00")
        ttk.Label(panel, textvariable=self.stats_text, foreground="#666", wraplength=300).grid(row=r, column=0, sticky="w", pady=(8,0)); r += 1

        self.status = tk.StringVar(value="提示：点击画布添加天体；所有飞出边界的天体会被删除。")
        ttk.Label(panel, textvariable=self.status, foreground="#666").grid(row=r, column=0, sticky="w", pady=(8,0)); r += 1

        ttk.Label(panel, text="天体数据（实时）", font=("Microsoft YaHei", 10, "bold")).grid(row=r, column=0, sticky="w", pady=(6,2)); r += 1
        cols = ("idx", "x", "y", "vx", "vy", "Δvx", "Δvy", "speed", "mass", "diam", "frag")
        self.tree = ttk.Treeview(panel, columns=cols, show="headings", height=10)
        self.tree.grid(row=r, column=0, sticky="nsew")
        for c, t in zip(cols, ("ID","x","y","vx","vy","Δvx","Δvy","|v|","m","d","frag")):
            self.tree.heading(c, text=t)
        self.tree.column("idx", width=30, anchor="center")
        for c in ("x","y","vx","vy","Δvx","Δvy","speed"):
            self.tree.column(c, width=60, anchor="e")
        self.tree.column("mass", width=60, anchor="e")
        self.tree.column("diam", width=50, anchor="e")
        self.tree.column("frag", width=40, anchor="center")
        scrollb = ttk.Scrollbar(panel, orient="vertical", command=self.tree.yview)
        self.tree.configure(yscrollcommand=scrollb.set)
        scrollb.grid(row=r, column=1, sticky="ns", pady=2)
        r += 1

    def on_canvas_click(self, event):
        xw, yw = self.screen_to_world(event.x, event.y)
        m = max(1e-6, float(self.new_mass.get()))
        d = max(1e-6, float(self.new_diam.get()))
        vx = float(self.init_vx.get())
        vy = float(self.init_vy.get())
        # 用户添加的一般天体不是碎片
        self.sim.add_body(xw, yw, vx, vy, m, d, is_fragment=False)
        self.status.set(f"已添加天体: pos=({xw:.2f},{yw:.2f}), v=({vx:.2f},{vy:.2f}), m={m:.2f}, d={d:.2f}")
        self._draw()

    def toggle_run(self):
        self.running = not self.running
        self.status.set("运行中…" if self.running else "已暂停")

    def single_step(self):
        self._simulate_one_step()
        self._draw()

    def reset(self):
        self.running = False
        self.sim.clear()
        self.status.set("已重置。")
        self._draw()

    def clear_bodies(self):
        self.sim.clear()
        self.status.set("已清空所有天体。")
        self._draw()

    def load_example_binary(self):
        self.reset()
        m = 50.0; d = 2.0; r = 15.0
        v = math.sqrt(G * m / (4.0 * r))
        self.sim.add_body(-r, 0.0, 0.0,  v, m, d, is_fragment=False)
        self.sim.add_body( r, 0.0, 0.0, -v, m, d, is_fragment=False)
        self.status.set("示例：等质量双星系统。")
        self._draw()

    def load_example_three(self):
        self.reset()
        m = 40.0; d = 2.0
        self.sim.add_body(-10.0,  0.0,  0.0,  0.8, m, d, is_fragment=False)
        self.sim.add_body( 10.0,  0.0,  0.0, -0.8, m, d, is_fragment=False)
        self.sim.add_body(  0.0, 17.0, -0.9,  0.0, m, d, is_fragment=False)
        self.status.set("示例：主的运动。")
        self._draw()

    def load_example_slingshot(self):
        # 引力弹弓示例：一个大质量“行星”在原点，一个小探测器从远处飞来近掠后被弹出
        self.reset()
        m_planet = 400.0
        d_planet = 5.0
        # 放在原点的静止大质量天体
        self.sim.add_body(0.0, 0.0, 0.0, 0.0, m_planet, d_planet, is_fragment=False)

        # 小探测器起于左侧偏下并朝右飞，略微向上，以便近掠
        m_probe = 1.0
        d_probe = 0.8
        start_x = -49.9
        start_y = -13.6
        vx_probe = 4.1   # 初始速度，靠得太慢会被捕获，太快则偏转太小
        vy_probe = 0.1
        self.sim.add_body(start_x, start_y, vx_probe, vy_probe, m_probe, d_probe, is_fragment=False)

        # 可再加些轻微背景天体（可选）
        self.status.set("示例：引力弹弓")
        self._draw()

    # ---------- 碰撞与碎片化（更节制） ----------
    def resolve_collision(self, idx_a, idx_b):
        if idx_a == idx_b:
            return

        # 读取属性
        ma = float(self.sim.mass[idx_a, 0]); mb = float(self.sim.mass[idx_b, 0])
        pa = np.array(self.sim.pos[idx_a]); pb = np.array(self.sim.pos[idx_b])
        va = np.array(self.sim.vel[idx_a]); vb = np.array(self.sim.vel[idx_b])
        da = float(self.sim.diam[idx_a, 0]); db = float(self.sim.diam[idx_b, 0])

        # 标记大/小体
        if ma >= mb:
            big_idx, small_idx = idx_a, idx_b
            m_big, m_small = ma, mb
            p_big, p_small = pa.copy(), pb.copy()
            v_big, v_small = va.copy(), vb.copy()
            d_big, d_small = da, db
        else:
            big_idx, small_idx = idx_b, idx_a
            m_big, m_small = mb, ma
            p_big, p_small = pb.copy(), pa.copy()
            v_big, v_small = vb.copy(), va.copy()
            d_big, d_small = db, da

        total_mass = m_big + m_small
        com_pos = (m_big * p_big + m_small * p_small) / total_mass
        com_vel = (m_big * v_big + m_small * v_small) / total_mass
        sep_vec = p_small - p_big
        sep = np.linalg.norm(sep_vec) + 1e-12
        rel_speed = np.linalg.norm(v_big - v_small)

        # Roche 判定
        vol_big = math.pi / 6.0 * (d_big ** 3)
        vol_small = math.pi / 6.0 * (d_small ** 3)
        rho_big = m_big / vol_big if vol_big > 0 else 1.0
        rho_small = m_small / vol_small if vol_small > 0 else 1.0
        roche_k = float(self.roche_k.get())  # 从滑块获取洛希极限系数
        R_big = d_big / 2.0
        roche_limit = roche_k * R_big * (rho_big / (rho_small + 1e-12)) ** (1.0/3.0)

        # 如果进入 Roche，则小体被撕碎（生成有限碎片）
        if sep < roche_limit:
            self._shred_small_into_fragments(big_idx, small_idx, p_big, v_big, m_big, d_big,
                                             p_small, v_small, m_small, d_small, com_vel, rel_speed)
            return

        # 吸收判据：若质量悬殊则吸收（保留）
        absorb_thresh = 0.05
        mass_ratio = m_small / m_big
        if mass_ratio <= absorb_thresh:
            merged_mass = total_mass
            merged_pos = com_pos
            merged_vel = com_vel
            merged_d = max(0.05, (merged_mass ** (1.0/3.0)) * 0.6)
            self.sim.delete_indices([big_idx, small_idx])
            self.sim.add_body(float(merged_pos[0]), float(merged_pos[1]),
                              float(merged_vel[0]), float(merged_vel[1]),
                              float(merged_mass), float(merged_d), is_fragment=False)
            return

        # 否则较为接近质量或中等质量差，进行受限碎裂/剥离（生成较少碎片）
        self._fragment_collision_limited(big_idx, small_idx, p_big, v_big, m_big, d_big,
                                         p_small, v_small, m_small, d_small, rel_speed)

    def _shred_small_into_fragments(self, big_idx, small_idx,
                                    p_big, v_big, m_big, d_big,
                                    p_small, v_small, m_small, d_small,
                                    com_vel, rel_speed):
        # 更节制的碎片分配：碎片数受限（<= small_frag_scale_limit）
        n_frag = int(min(self.small_frag_scale_limit, max(2, round(m_small / 0.5))))
        n_frag = max(2, min(n_frag, self.small_frag_scale_limit))
        weights = self.rng.random(n_frag)
        weights = np.maximum(weights, 1e-6)
        masses = (weights / weights.sum()) * m_small
        masses = masses[masses >= self.min_frag_mass]
        if len(masses) == 0:
            masses = np.array([m_small])
        frag_positions = []
        frag_vels = []
        for mfrag in masses:
            ang = self.rng.uniform(0, 2*math.pi)
            r_off = 0.2 * (mfrag ** (1/3))
            pos = p_small + np.array([math.cos(ang), math.sin(ang)]) * r_off
            eject = (rel_speed * 0.6 + 0.15) * (1.0 + 0.3 * self.rng.random())
            dir_noise = np.array([math.cos(ang), math.sin(ang)]) + 0.2 * self.rng.normal(size=2)
            dir_noise /= (np.linalg.norm(dir_noise) + 1e-12)
            vel = com_vel + dir_noise * eject
            frag_positions.append(pos)
            frag_vels.append(vel)

        # 删除原两体，重新加入大体并加入碎片（碎片标记为 is_fragment=True）
        self.sim.delete_indices([big_idx, small_idx])
        d_big_new = max(0.05, (m_big ** (1.0/3.0)) * 0.6)
        self.sim.add_body(float(p_big[0]), float(p_big[1]), float(v_big[0]), float(v_big[1]), float(m_big), float(d_big_new), is_fragment=False)
        for mfrag, pfrag, vfrag in zip(masses, frag_positions, frag_vels):
            dfrag = max(0.05, (mfrag ** (1.0/3.0)) * 0.6)
            self.sim.add_body(float(pfrag[0]), float(pfrag[1]), float(vfrag[0]), float(vfrag[1]), float(mfrag), float(dfrag), is_fragment=True)

    def _fragment_collision_limited(self, big_idx, small_idx,
                                    p_big, v_big, m_big, d_big,
                                    p_small, v_small, m_small, d_small,
                                    rel_speed):
        total_mass = m_big + m_small
        com_pos = (m_big * p_big + m_small * p_small) / total_mass
        com_vel = (m_big * v_big + m_small * v_small) / total_mass
        mass_ratio = m_small / m_big

        fragments_pos = []
        fragments_vel = []
        fragments_mass = []

        # 若接近质量则生成少量碎片，否则小体粉碎、主体剥离少量
        if mass_ratio >= self.similar_thresh:
            approx_n = int(min(int(self.max_total_frags.get()), max(3, round(total_mass / 4.0))))  # 从滑块获取最大碎片数量
            weights = self.rng.random(approx_n)
            weights = np.maximum(weights, 1e-6)
            masses = (weights / weights.sum()) * total_mass
            masses = masses[masses >= self.min_frag_mass]
            if len(masses) == 0:
                masses = np.array([total_mass])
            for mfrag in masses:
                ang = self.rng.uniform(0, 2*math.pi)
                pos = com_pos + 0.1 * np.array([math.cos(ang), math.sin(ang)]) * (mfrag ** (1/3))
                eject = (rel_speed * 0.4 + 0.1) * (1.0 + 0.8 * (1.0 - mfrag/total_mass))
                dir_noise = self.rng.normal(size=2)
                dir_noise /= (np.linalg.norm(dir_noise) + 1e-12)
                vel = com_vel + dir_noise * eject
                fragments_pos.append(pos); fragments_vel.append(vel); fragments_mass.append(mfrag)
        else:
            # 小体粉碎（受限数量），主体剥离少量
            n_small = int(min(self.small_frag_scale_limit, max(2, round(m_small / 0.6))))
            weights = self.rng.random(n_small); weights = np.maximum(weights, 1e-6)
            masses_small = (weights / weights.sum()) * m_small
            masses_small = masses_small[masses_small >= self.min_frag_mass]
            if len(masses_small) == 0:
                masses_small = np.array([m_small])
            for mfrag in masses_small:
                ang = self.rng.uniform(0, 2*math.pi)
                pos = (p_big + p_small) / 2.0 + np.array([math.cos(ang), math.sin(ang)]) * 0.15 * (mfrag ** (1/3))
                eject = (rel_speed * 0.6 + 0.15) * (1.0 + self.rng.random()*0.4)
                dir_noise = np.array([math.cos(ang), math.sin(ang)]) + 0.2 * self.rng.normal(size=2)
                dir_noise /= (np.linalg.norm(dir_noise) + 1e-12)
                vel = com_vel + dir_noise * eject
                fragments_pos.append(pos); fragments_vel.append(vel); fragments_mass.append(mfrag)
            eject_from_big = min(0.15 * m_big, 0.4 * m_small)
            big_rem_mass = m_big - eject_from_big
            if big_rem_mass < self.min_frag_mass:
                # 如果主残余太小则直接全部碎裂为少量碎片
                self._full_shatter_and_add(total_mass, com_pos, com_vel, rel_speed)
                self.sim.delete_indices([big_idx, small_idx])
                return
            # 添加主残余和一块被剥离小体
            fragments_pos.append(com_pos); fragments_vel.append(com_vel); fragments_mass.append(big_rem_mass)
            ang = self.rng.uniform(0, 2*math.pi)
            pos = com_pos + np.array([math.cos(ang), math.sin(ang)]) * 0.3 * (eject_from_big ** (1/3))
            dir_noise = self.rng.normal(size=2); dir_noise /= (np.linalg.norm(dir_noise) + 1e-12)
            vel = com_vel + dir_noise * (rel_speed * 0.4 + 0.1)
            fragments_pos.append(pos); fragments_vel.append(vel); fragments_mass.append(eject_from_big)

        # 合并微小碎片并限制总数
        fr_mass = np.array(fragments_mass)
        mask = fr_mass >= self.min_frag_mass
        if not np.any(mask):
            fragments_pos = [com_pos]; fragments_vel = [com_vel]; fragments_mass = [total_mass]
        else:
            fragments_pos = [fragments_pos[k] for k in range(len(fragments_mass)) if mask[k]]
            fragments_vel = [fragments_vel[k] for k in range(len(fragments_mass)) if mask[k]]
            fragments_mass = [fragments_mass[k] for k in range(len(fragments_mass)) if mask[k]]

        max_frags = int(self.max_total_frags.get())  # 从滑块获取最大碎片数量
        while len(fragments_mass) > max_frags:
            idxs = np.argsort(fragments_mass)
            a, b = idxs[0], idxs[1]
            ma, mb = fragments_mass[a], fragments_mass[b]
            pa, pb = fragments_pos[a], fragments_pos[b]
            va, vb = fragments_vel[a], fragments_vel[b]
            m_new = ma + mb
            p_new = (ma * pa + mb * pb) / m_new
            v_new = (ma * va + mb * vb) / m_new
            new_pos = []; new_vel = []; new_mass = []
            for k in range(len(fragments_mass)):
                if k in (a, b): continue
                new_pos.append(fragments_pos[k]); new_vel.append(fragments_vel[k]); new_mass.append(fragments_mass[k])
            new_pos.append(p_new); new_vel.append(v_new); new_mass.append(m_new)
            fragments_pos = new_pos; fragments_vel = new_vel; fragments_mass = new_mass

        # 动量修正以近似守恒
        pre_mom = m_big * v_big + m_small * v_small
        frag_masses = np.array(fragments_mass)
        frag_vels = np.array(fragments_vel)
        frag_mom = (frag_masses[:, None] * frag_vels).sum(axis=0)
        mom_diff = pre_mom - frag_mom
        if frag_masses.sum() > 0:
            vel_correction = mom_diff / frag_masses.sum()
            frag_vels = frag_vels + vel_correction[None, :]

        # 删除原两体并加入碎片（标记为碎片）
        self.sim.delete_indices([big_idx, small_idx])
        for mfrag, pfrag, vfrag in zip(frag_masses, fragments_pos, frag_vels):
            dfrag = max(0.05, (mfrag ** (1.0/3.0)) * 0.6)
            self.sim.add_body(float(pfrag[0]), float(pfrag[1]), float(vfrag[0]), float(vfrag[1]), float(mfrag), float(dfrag), is_fragment=True)

    def _full_shatter_and_add(self, total_mass, com_pos, com_vel, rel_speed):
        max_frag = min(int(self.max_total_frags.get()), 4)  # 从滑块获取最大碎片数量
        weights = self.rng.random(max_frag)
        weights = np.maximum(weights, 1e-6)
        masses = (weights / weights.sum()) * total_mass
        for mfrag in masses:
            ang = self.rng.uniform(0, 2*math.pi)
            pos = com_pos + 0.1 * np.array([math.cos(ang), math.sin(ang)]) * (mfrag ** (1/3))
            dir_noise = self.rng.normal(size=2); dir_noise /= (np.linalg.norm(dir_noise) + 1e-12)
            vel = com_vel + dir_noise * (rel_speed * 0.4 + 0.1)
            dfrag = max(0.05, (mfrag ** (1.0/3.0)) * 0.6)
            self.sim.add_body(float(pos[0]), float(pos[1]), float(vel[0]), float(vel[1]), float(mfrag), float(dfrag), is_fragment=True)

    # ---------- 主循环 ----------
    def _schedule_loop(self):
        self.root.after(16, self._loop)

    def _loop(self):
        if self.running:
            self._simulate_one_step()
            self._draw()
        self._schedule_loop()

    def _simulate_one_step(self):
        if self.sim.n == 0:
            return
        dt = max(1e-5, float(self.dt.get()))
        L = max(1.0, float(self.box_L.get()))
        # 推进一步（不反射），并在后续删除所有飞出边界的天体
        self.sim.step_leapfrog(dt, L, reflect=False)

        # 删除所有飞出边界的天体（包括碎片和普通天体）
        out_mask_x = np.abs(self.sim.pos[:,0]) > L
        out_mask_y = np.abs(self.sim.pos[:,1]) > L
        out_mask = np.logical_or(out_mask_x, out_mask_y)
        if np.any(out_mask):
            out_idxs = np.where(out_mask)[0]
            # 报告数量前保存大小，delete_indices 会改变 sim.n
            n_removed = out_idxs.size
            self.sim.delete_indices(out_idxs)
            self.status.set(f"删除了 {n_removed} 个飞出边界的天体。")

        # 处理碰撞（限制单步处理次数）
        max_collisions_per_step = 6
        handled = 0
        while True:
            collided, info = self.sim.detect_collision()
            if not collided:
                break
            i, j, dist = info
            try:
                self.resolve_collision(i, j)
            except Exception as e:
                # 保护性删除以避免死循环
                idxs = sorted([i, j], reverse=True)
                self.sim.delete_indices(idxs)
                self.status.set(f"碰撞处理异常，已移除：{e}")
            handled += 1
            if handled >= max_collisions_per_step:
                break

    # ---------- 绘制与表格 ----------
    def _draw(self):
        self.canvas.delete("all")
        L = float(self.box_L.get())
        x0, y0 = self.world_to_screen(-L, -L)
        x1, y1 = self.world_to_screen( L,  L)
        self.canvas.create_rectangle(x0, y1, x1, y0, outline="#444")

        if self.sim.n > 0:
            k = self.canvas_size / (2.0 * L)
            for idx in range(self.sim.n):
                x, y = self.sim.pos[idx]
                d_world = float(self.sim.diam[idx, 0])
                d_px = max(2.0, d_world * k)
                cx, cy = self.world_to_screen(x, y)
                r = d_px / 2.0
                m = float(self.sim.mass[idx, 0])
                if self.sim.is_frag[idx,0]:
                    color = "#f9a"  # 碎片上色为粉色系
                else:
                    shade = int(min(255, 60 + m))
                    color = f"#{shade:02x}{200:02x}{255:02x}"
                self.canvas.create_oval(cx - r, cy - r, cx + r, cy + r, fill=color, outline="")

        txt = f"t={self.sim.t:.2f}  N={self.sim.n}  dt={float(self.dt.get()):.4f}  L={L:.1f}"
        self.canvas.create_text(8, 8, text=txt, fill="#ddd", anchor="nw")

        self.update_table()
        self.update_stats()

    def update_stats(self):
        if self.sim.n == 0:
            self.stats_text.set("天体: 0, 碎片: 0 | 天体最大速度: 0.00, 天体最大加速度: 0.00, 天体最大质量: 0.00 | 碎片最大速度: 0.00")
            return
            
        # 计算统计信息
        n_total = self.sim.n
        n_frag = np.sum(self.sim.is_frag[:,0])
        n_body = n_total - n_frag
        
        # 计算速度
        speeds = np.linalg.norm(self.sim.vel, axis=1)
        max_speed = np.max(speeds) if n_total > 0 else 0.0
        
        # 计算加速度
        acc_mags = np.linalg.norm(self.sim.acc, axis=1)
        max_acc = np.max(acc_mags) if n_total > 0 else 0.0
        
        # 计算质量
        masses = self.sim.mass[:,0]
        max_mass = np.max(masses) if n_total > 0 else 0.0
        
        # 计算碎片最大速度
        if n_frag > 0:
            frag_speeds = speeds[self.sim.is_frag[:,0]]
            max_frag_speed = np.max(frag_speeds)
        else:
            max_frag_speed = 0.0
        
        self.stats_text.set(f"天体: {n_body}, 碎片: {n_frag} | 天体最大速度: {max_speed:.2f}, 天体最大加速度: {max_acc:.2f}, 天体最大质量: {max_mass:.2f} | 碎片最大速度: {max_frag_speed:.2f}")

    def update_table(self):
        for it in self.tree.get_children():
            self.tree.delete(it)
        for idx in range(self.sim.n):
            x, y = self.sim.pos[idx]
            vx, vy = self.sim.vel[idx]
            # 计算Δv（速度变化量）
            prev_vx, prev_vy = self.sim.prev_vel[idx]
            delta_vx = vx - prev_vx
            delta_vy = vy - prev_vy
            speed = math.hypot(vx, vy)
            m = float(self.sim.mass[idx, 0])
            d = float(self.sim.diam[idx, 0])
            frag = "Y" if self.sim.is_frag[idx,0] else "N"
            self.tree.insert("", "end", values=(
                idx,
                f"{x:.3f}", f"{y:.3f}",
                f"{vx:.3f}", f"{vy:.3f}",
                f"{delta_vx:.3f}", f"{delta_vy:.3f}",
                f"{speed:.3f}",
                f"{m:.3f}", f"{d:.3f}", frag
            ))

    def world_to_screen(self, xw, yw):
        L = float(self.box_L.get())
        k = self.canvas_size / (2.0 * L)
        sx = (xw + L) * k
        sy = (L - yw) * k
        return sx, sy

    def screen_to_world(self, sx, sy):
        L = float(self.box_L.get())
        k = self.canvas_size / (2.0 * L)
        xw = sx / k - L
        yw = L - sy / k
        return xw, yw

if __name__ == "__main__":
    root = tk.Tk()
    try:
        root.iconbitmap("")
    except Exception:
        pass
    style = ttk.Style()
    try:
        style.theme_use("clam")
    except tk.TclError:
        pass
    app = NBodyApp(root)
    root.mainloop()
