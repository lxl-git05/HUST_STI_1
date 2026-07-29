"""
Y8 寻迹串口数据可视化 — 双击 run.bat 即可运行
支持两种格式:
  新(8列): Y8_Bias, SetPoint, Gyro, Yaw, SpeedA, SpeedB, Y8_Byte, BaseSpeed
  旧(4列): goalPoint, realPoint_Now, setPoint, Yaw_Abs
"""
import sys, os, re
import numpy as np
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
from matplotlib.gridspec import GridSpec

# 中文字体 (Windows)
matplotlib.rcParams['font.sans-serif'] = ['Microsoft YaHei', 'SimHei', 'DejaVu Sans']
matplotlib.rcParams['axes.unicode_minus'] = False

# ===== Y8_Driver.c 当前参数 =====
DEADBAND = 2.7
DEADBAND_X2 = 5.4
GYRO_STRAIGHT = 5.0
GYRO_CURVE = 20.0
KP, KD = 4.52, 5.0
DT = 0.02  # 20ms

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
DATA_FILE = os.path.join(SCRIPT_DIR, 'data.txt')
OUT_FILE  = os.path.join(SCRIPT_DIR, 'analysis.png')

def load_data(path):
    """读取CSV, 自动适配4列(旧)和8列(新)格式"""
    raw = []
    with open(path, 'r', encoding='utf-8', errors='ignore') as f:
        for line in f:
            line = line.strip()
            if not line: continue
            # 跳过非数据行
            parts = line.split(',')
            if len(parts) < 4: continue
            try:
                nums = [float(p.strip()) for p in parts[:8]]
            except ValueError:
                continue
            raw.append(nums)

    if not raw:
        raise ValueError(f'No valid data found in {path}')

    raw = np.array(raw, dtype=np.float32)
    cols = raw.shape[1]

    if cols >= 8:
        # 新格式
        return {
            'Y8_Bias': raw[:,0], 'SetPoint': raw[:,1], 'Gyro': raw[:,2],
            'Yaw': raw[:,3], 'SpeedA': raw[:,4], 'SpeedB': raw[:,5],
            'Y8_Byte': raw[:,6], 'BaseSpeed': raw[:,7]
        }
    else:
        # 旧格式: goalPoint, realPoint_Now, setPoint, Yaw_Abs
        return {
            'Y8_Bias': raw[:,1],        # realPoint_Now → 作为角度
            'SetPoint': raw[:,2],        # setPoint → PID输出
            'Gyro': np.zeros(len(raw)),  # 旧格式无角速度
            'Yaw': raw[:,3],             # Yaw_Abs
            'SpeedA': np.zeros(len(raw)),
            'SpeedB': np.zeros(len(raw)),
            'Y8_Byte': np.zeros(len(raw)),  # 旧格式无传感器
            'BaseSpeed': np.zeros(len(raw)),
        }

def decode_y8(byte_vals):
    bits = np.zeros((len(byte_vals), 8), dtype=np.int8)
    for i in range(8):
        bits[:, i] = ((byte_vals.astype(int) >> i) & 1)
    return bits

def plot_all(data, title_extra=''):
    n = len(data['Y8_Bias'])
    t = np.arange(n) * DT
    y8_bits = decode_y8(data['Y8_Byte'])
    total_diff = data['SpeedA'] - data['SpeedB']
    abs_bias = np.abs(data['Y8_Bias'])
    has_gyro = np.any(data['Gyro'] != 0)
    has_motor = np.any(data['SpeedA'] != 0) or np.any(data['SpeedB'] != 0)
    has_y8 = np.any(data['Y8_Byte'] != 0)

    fig = plt.figure(figsize=(18, 13))
    gs = GridSpec(4, 2, figure=fig, hspace=0.38, wspace=0.28)

    # (0,0) Y8_Bias
    ax = fig.add_subplot(gs[0, 0])
    ax.plot(t, data['Y8_Bias'], 'b-', linewidth=0.8, label='Y8_Bias (°)')
    ax.axhline(DEADBAND, color='orange', ls='--', alpha=0.5, label=f'Deadband ±{DEADBAND}°')
    ax.axhline(-DEADBAND, color='orange', ls='--', alpha=0.5)
    ax.axhline(DEADBAND_X2, color='red', ls=':', alpha=0.3, label=f'Smoothstep end ±{DEADBAND_X2}°')
    ax.axhline(-DEADBAND_X2, color='red', ls=':', alpha=0.3)
    ax.axhline(0, color='gray', ls='-', alpha=0.2)
    ax.set_ylabel('Angle (°)'); ax.set_title('Y8_Bias (Filtered Angle)')
    ax.legend(fontsize=7, loc='upper right'); ax.grid(True, alpha=0.3)

    # (0,1) Y8 热力图 或 空
    ax = fig.add_subplot(gs[0, 1])
    if has_y8:
        labels = ['-42', '-30', '-18', '-6', '+6', '+18', '+30', '+42']
        ax.imshow(y8_bits.T, aspect='auto', cmap='RdYlGn_r', interpolation='nearest', vmin=0, vmax=1)
        ax.set_yticks(range(8)); ax.set_yticklabels(labels, fontsize=7)
        ax.set_title('Y8 Sensors (Green=Line, Red=White)')
    else:
        ax.text(0.5, 0.5, 'No Y8 sensor data\n(旧格式或全丢线)', ha='center', va='center',
                transform=ax.transAxes, fontsize=12, color='gray')
        ax.set_title('Y8 Sensors (N/A)')
    ax.set_xlabel('Time (s)')

    # (1,0) SetPoint + 实际差速
    ax = fig.add_subplot(gs[1, 0])
    ax.plot(t, data['SetPoint'], 'r-', linewidth=0.8, label='PID SetPoint (rpm)')
    if has_motor:
        ax.plot(t, total_diff, 'purple', linewidth=0.8, alpha=0.5, label='Actual diff (A-B)')
    ax.axhline(60, color='gray', ls='--', alpha=0.3); ax.axhline(-60, color='gray', ls='--', alpha=0.3)
    ax.axhline(0, color='gray', ls='-', alpha=0.2)
    ax.set_ylabel('Differential (rpm)'); ax.set_title('Steering Command (Red) + Actual Diff (Purple)')
    ax.legend(fontsize=7); ax.grid(True, alpha=0.3)

    # (1,1) SetPoint vs Bias 散点 (仅当有角速度时着色)
    ax = fig.add_subplot(gs[1, 1])
    if has_gyro:
        colors = np.where(np.abs(data['Gyro']) < GYRO_STRAIGHT, 'blue',
                   np.where(np.abs(data['Gyro']) > GYRO_CURVE, 'red', 'green'))
        ax.scatter(data['Y8_Bias'], data['SetPoint'], c=colors, s=3, alpha=0.5)
        from matplotlib.patches import Patch
        ax.legend(handles=[
            Patch(color='blue', label='Straight (<5°/s)'),
            Patch(color='green', label='Transition (5-20°/s)'),
            Patch(color='red', label='Curve (>20°/s)'),
        ], fontsize=7)
    else:
        ax.scatter(data['Y8_Bias'], data['SetPoint'], s=3, alpha=0.5, color='gray')
    ax.axvline(DEADBAND, color='orange', ls='--', alpha=0.3)
    ax.axvline(-DEADBAND, color='orange', ls='--', alpha=0.3)
    ax.set_xlabel('Y8_Bias (°)'); ax.set_ylabel('SetPoint (rpm)')
    ax.set_title('SetPoint vs Bias'); ax.grid(True, alpha=0.3)

    # (2,0) IMU 角速度
    ax = fig.add_subplot(gs[2, 0])
    if has_gyro:
        ax.fill_between(t, 0, data['Gyro'], alpha=0.3, color='green')
        ax.plot(t, data['Gyro'], 'g-', linewidth=0.8, label='Gyro Z (°/s)')
        ax.axhline(GYRO_STRAIGHT, color='blue', ls='--', alpha=0.5, label=f'Straight <{GYRO_STRAIGHT}°/s')
        ax.axhline(GYRO_CURVE, color='red', ls='--', alpha=0.5, label=f'Curve >{GYRO_CURVE}°/s')
        ax.legend(fontsize=7)
    else:
        ax.text(0.5, 0.5, 'No Gyro data (旧格式)', ha='center', va='center',
                transform=ax.transAxes, fontsize=12, color='gray')
    ax.set_ylabel('Yaw Rate (°/s)'); ax.set_title('IMU Gyro Z — Curve/Straight Detection')
    ax.grid(True, alpha=0.3)

    # (2,1) 偏航 + 电机
    ax = fig.add_subplot(gs[2, 1])
    ax.plot(t, data['Yaw'], 'b-', linewidth=0.8, label='Yaw Abs (°)')
    ax.set_ylabel('Yaw (°)', color='blue')
    if has_motor:
        ax2 = ax.twinx()
        ax2.plot(t, data['SpeedA'], 'r-', linewidth=0.5, alpha=0.6, label='Motor A')
        ax2.plot(t, data['SpeedB'], 'orange', linewidth=0.5, alpha=0.6, label='Motor B')
        ax2.set_ylabel('Speed (rpm)', color='red')
    ax.set_title('Heading + Motor Speeds')
    lines1, labels1 = ax.get_legend_handles_labels()
    if has_motor:
        lines2, labels2 = ax2.get_legend_handles_labels()
        ax.legend(lines1+lines2, labels1+labels2, fontsize=7)
    else:
        ax.legend(fontsize=7)
    ax.grid(True, alpha=0.3)

    # (3,0) 统计
    ax = fig.add_subplot(gs[3, 0])
    ax.axis('off')
    stats = [
        f"Duration: {n*DT:.1f}s  ({n} samples @ {1/DT:.0f}Hz)",
        f"Y8_Bias:  mean(|bias|)={np.mean(abs_bias):.2f}  max={np.max(abs_bias):.2f}  std={np.std(data['Y8_Bias']):.2f}°",
        f"SetPoint: mean(|sp|)={np.mean(np.abs(data['SetPoint'])):.1f}  max={np.max(np.abs(data['SetPoint'])):.1f}  std={np.std(data['SetPoint']):.1f} rpm",
    ]
    if has_gyro:
        stats.append(f"Gyro:     mean(|gyro|)={np.mean(np.abs(data['Gyro'])):.1f}  max(|gyro|)={np.max(np.abs(data['Gyro'])):.1f} °/s")
    if has_motor:
        stats.append(f"SpeedA:   mean={np.mean(data['SpeedA']):.1f}  max={np.max(data['SpeedA']):.1f} rpm | SpeedB: mean={np.mean(data['SpeedB']):.1f}  max={np.max(data['SpeedB']):.1f} rpm")
    if has_y8:
        stats.append(f"Y8 active: mean={np.mean(np.sum(y8_bits, axis=1)):.1f}/8 sensors  loss: {(data['Y8_Byte']==0).sum()/n*100:.0f}%")
    if has_gyro:
        p_st = (np.abs(data['Gyro'])<GYRO_STRAIGHT).sum()/n*100
        p_cv = (np.abs(data['Gyro'])>GYRO_CURVE).sum()/n*100
        p_tr = 100 - p_st - p_cv
        stats.append(f"Mode:    Straight {p_st:.0f}% | Transition {p_tr:.0f}% | Curve {p_cv:.0f}%")
    stats.append(f"Deadband: within ±{DEADBAND}° {(abs_bias<DEADBAND).sum()/n*100:.0f}%  |  >{DEADBAND_X2}° {(abs_bias>DEADBAND_X2).sum()/n*100:.0f}%")
    stats.append(f"SetPoint saturated (|SP|>58): {(np.abs(data['SetPoint'])>58).sum()/n*100:.0f}%")
    for i, s in enumerate(stats):
        ax.text(0.05, 0.95 - i*0.11, s, transform=ax.transAxes, fontsize=8, fontfamily='monospace', verticalalignment='top')

    # (3,1) |Bias| 分布
    ax = fig.add_subplot(gs[3, 1])
    ax.hist(abs_bias, bins=40, color='steelblue', edgecolor='white', alpha=0.8)
    ax.axvline(DEADBAND, color='orange', ls='--', lw=1.5, label=f'Deadband {DEADBAND}°')
    ax.axvline(DEADBAND_X2, color='red', ls=':', lw=1.5, label=f'Smoothstep end {DEADBAND_X2}°')
    ax.set_xlabel('|Y8_Bias| (°)'); ax.set_ylabel('Count')
    ax.set_title('Bias Magnitude Distribution'); ax.legend(fontsize=7); ax.grid(True, alpha=0.3)

    title = f'Y8 Tracking  |  {n} samples, {n*DT:.1f}s  |  {title_extra}'.rstrip(' |')
    fig.suptitle(title, fontsize=13, fontweight='bold', y=0.99)
    fig.savefig(OUT_FILE, dpi=150, bbox_inches='tight', facecolor='white')
    print(f'OK: {OUT_FILE}  ({n} samples, {n*DT:.1f}s)')
    plt.close(fig)

if __name__ == '__main__':
    os.chdir(SCRIPT_DIR)
    extra = sys.argv[1] if len(sys.argv) > 1 else ''
    d = load_data(DATA_FILE)
    plot_all(d, extra)
