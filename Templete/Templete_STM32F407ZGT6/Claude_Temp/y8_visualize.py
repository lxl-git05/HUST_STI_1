"""
Y8 寻迹串口数据可视化
用法: D:\conda_envs\claude_env\python.exe y8_visualize.py data.csv [--title "描述"]
串口格式: Y8_Bias, SetPoint, Gyro, Yaw, SpeedA, SpeedB, Y8_Byte, BaseSpeed
"""
import sys, os
import numpy as np
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
from matplotlib.gridspec import GridSpec

# ===== 参数 (与 Y8_Driver.c 保持一致) =====
DEADBAND = 2.7          # 软死区 (°)
DEADBAND_X2 = 5.4       # smoothstep 过渡区上限
GYRO_STRAIGHT = 5.0     # 直道判定阈值 (°/s)
GYRO_CURVE = 20.0       # 弯道判定阈值 (°/s)
KP, KD = 4.52, 5.0      # PID 增益

def load_csv(path):
    data = np.loadtxt(path, delimiter=',', dtype=np.float32)
    if data.ndim == 1:
        data = data.reshape(1, -1)
    cols = data.shape[1]
    names = ['Y8_Bias', 'SetPoint', 'Gyro', 'Yaw', 'SpeedA', 'SpeedB', 'Y8_Byte', 'BaseSpeed'][:cols]
    return {n: data[:, i] for i, n in enumerate(names)}, data.shape[0]

def decode_y8(byte_val):
    """将 Y8_Byte 解码为 8 路传感器状态 (1=黑线)"""
    bits = np.zeros((len(byte_val), 8), dtype=np.int8)
    for i in range(8):
        bits[:, i] = ((byte_val.astype(int) >> i) & 1)
    return bits

def main():
    if len(sys.argv) < 2:
        print("用法: python y8_visualize.py data.csv [--title 描述]")
        print("CSV 列: Y8_Bias, SetPoint, Gyro, Yaw, SpeedA, SpeedB, Y8_Byte, BaseSpeed")
        sys.exit(1)

    path = sys.argv[1]
    title_extra = ' '.join(sys.argv[2:]).replace('--title ', '')

    d, n = load_csv(path)
    t = np.arange(n) * 0.02  # 20ms 周期 → 秒
    y8_bits = decode_y8(d['Y8_Byte'])

    # 计算派生量
    total_diff = d['SpeedA'] - d['SpeedB']          # 实际差速
    abs_bias = np.abs(d['Y8_Bias'])

    fig = plt.figure(figsize=(16, 12))
    gs = GridSpec(4, 2, figure=fig, hspace=0.35, wspace=0.25)

    # ---- (0,0) Y8_Bias + 死区/smoothstep 参考线 ----
    ax = fig.add_subplot(gs[0, 0])
    ax.plot(t, d['Y8_Bias'], 'b-', linewidth=0.8, label='Y8_Bias (°)')
    ax.axhline(DEADBAND, color='orange', ls='--', alpha=0.5, label=f'Deadband ±{DEADBAND}°')
    ax.axhline(-DEADBAND, color='orange', ls='--', alpha=0.5)
    ax.axhline(DEADBAND_X2, color='red', ls=':', alpha=0.3, label=f'Smoothstep end ±{DEADBAND_X2}°')
    ax.axhline(-DEADBAND_X2, color='red', ls=':', alpha=0.3)
    ax.axhline(0, color='gray', ls='-', alpha=0.2)
    ax.set_ylabel('Angle (°)'); ax.set_title('Y8_Bias (Filtered Angle)'); ax.legend(fontsize=7); ax.grid(True, alpha=0.3)

    # ---- (0,1) Y8 传感器热力图 ----
    ax = fig.add_subplot(gs[0, 1])
    sensor_labels = ['-42', '-30', '-18', '-6', '+6', '+18', '+30', '+42']
    ax.imshow(y8_bits.T, aspect='auto', cmap='RdYlGn_r', interpolation='nearest', vmin=0, vmax=1)
    ax.set_yticks(range(8)); ax.set_yticklabels(sensor_labels, fontsize=7)
    ax.set_xlabel('Time (s)'); ax.set_ylabel('Sensor pos (mm)')
    ax.set_title('Y8 Sensors (Green=Black line detected, Red=White)')

    # ---- (1,0) PID SetPoint + 实际差速 ----
    ax = fig.add_subplot(gs[1, 0])
    ax.plot(t, d['SetPoint'], 'r-', linewidth=0.8, label='PID SetPoint (rpm)')
    ax.plot(t, total_diff, 'purple', linewidth=0.8, alpha=0.6, label='Actual diff (A-B rpm)')
    ax.axhline(60, color='gray', ls='--', alpha=0.3, label='±60 rpm limit')
    ax.axhline(-60, color='gray', ls='--', alpha=0.3)
    ax.axhline(0, color='gray', ls='-', alpha=0.2)
    ax.set_ylabel('Differential (rpm)'); ax.set_title('Steering Command vs Actual'); ax.legend(fontsize=7); ax.grid(True, alpha=0.3)

    # ---- (1,1) 输出限幅分析 (SetPoint vs Bias 散点) ----
    ax = fig.add_subplot(gs[1, 1])
    colors = np.where(np.abs(d['Gyro']) < GYRO_STRAIGHT, 'blue',        # 直道
               np.where(np.abs(d['Gyro']) > GYRO_CURVE, 'red', 'green'))  # 弯道/过渡
    ax.scatter(d['Y8_Bias'], d['SetPoint'], c=colors, s=3, alpha=0.5)
    ax.axvline(DEADBAND, color='orange', ls='--', alpha=0.3)
    ax.axvline(-DEADBAND, color='orange', ls='--', alpha=0.3)
    ax.set_xlabel('Y8_Bias (°)'); ax.set_ylabel('SetPoint (rpm)')
    ax.set_title('SetPoint vs Bias (Blue=Straight, Red=Curve, Green=Transition)')
    from matplotlib.patches import Patch
    ax.legend(handles=[
        Patch(color='blue', label='Straight (<5°/s)'),
        Patch(color='green', label='Transition (5-20°/s)'),
        Patch(color='red', label='Curve (>20°/s)'),
    ], fontsize=7); ax.grid(True, alpha=0.3)

    # ---- (2,0) IMU 角速度 + 直道/弯道阈值 ----
    ax = fig.add_subplot(gs[2, 0])
    ax.fill_between(t, 0, d['Gyro'], alpha=0.3, color='green')
    ax.plot(t, d['Gyro'], 'g-', linewidth=0.8, label='Gyro Z (°/s)')
    ax.axhline(GYRO_STRAIGHT, color='blue', ls='--', alpha=0.5, label=f'Straight thresh {GYRO_STRAIGHT}°/s')
    ax.axhline(GYRO_CURVE, color='red', ls='--', alpha=0.5, label=f'Curve thresh {GYRO_CURVE}°/s')
    ax.set_ylabel('Yaw Rate (°/s)'); ax.set_title('IMU Gyro Z (Curve/Straight Discrimination)')
    ax.legend(fontsize=7); ax.grid(True, alpha=0.3)

    # ---- (2,1) 绝对偏航角 + 电机转速 ----
    ax = fig.add_subplot(gs[2, 1])
    ax.plot(t, d['Yaw'], 'b-', linewidth=0.8, label='Yaw Abs (°)')
    ax.set_ylabel('Yaw (°)', color='blue')
    ax2 = ax.twinx()
    ax2.plot(t, d['SpeedA'], 'r-', linewidth=0.5, alpha=0.6, label='Motor A (rpm)')
    ax2.plot(t, d['SpeedB'], 'orange', linewidth=0.5, alpha=0.6, label='Motor B (rpm)')
    ax2.set_ylabel('Motor Speed (rpm)', color='red')
    ax.set_title('Heading + Motor Speeds')
    lines1, labels1 = ax.get_legend_handles_labels()
    lines2, labels2 = ax2.get_legend_handles_labels()
    ax.legend(lines1+lines2, labels1+labels2, fontsize=7); ax.grid(True, alpha=0.3)

    # ---- (3,0) 统计面板 ----
    ax = fig.add_subplot(gs[3, 0])
    ax.axis('off')
    stats = [
        f"Data points: {n}  ({n*0.02:.1f}s)",
        f"Y8_Bias:  mean={np.mean(abs_bias):.2f}°  max={np.max(abs_bias):.2f}°  std={np.std(d['Y8_Bias']):.2f}°",
        f"SetPoint: mean={np.mean(np.abs(d['SetPoint'])):.1f}  max={np.max(np.abs(d['SetPoint'])):.1f}  std={np.std(d['SetPoint']):.1f} rpm",
        f"Gyro:     mean={np.mean(d['Gyro']):.1f}  max={np.max(d['Gyro']):.1f} °/s",
        f"SpeedA:   mean={np.mean(d['SpeedA']):.1f} rpm   SpeedB: mean={np.mean(d['SpeedB']):.1f} rpm",
        f"Y8 active sensors: mean={np.mean(np.sum(y8_bits, axis=1)):.1f}/8",
        f"Time in straight: {(np.abs(d['Gyro'])<GYRO_STRAIGHT).sum()/n*100:.0f}%  "
        f"curve: {(np.abs(d['Gyro'])>GYRO_CURVE).sum()/n*100:.0f}%  "
        f"transition: {((np.abs(d['Gyro'])>=GYRO_STRAIGHT)&(np.abs(d['Gyro'])<=GYRO_CURVE)).sum()/n*100:.0f}%",
        f"Bias in deadband: {(abs_bias<DEADBAND).sum()/n*100:.0f}%  "
        f"Bias > 2x deadband: {(abs_bias>DEADBAND_X2).sum()/n*100:.0f}%",
        f"SetPoint saturated (|SP|>58): {(np.abs(d['SetPoint'])>58).sum()/n*100:.0f}%",
        f"Loss ticks (y8_byte==0): {(d['Y8_Byte']==0).sum()/n*100:.1f}%",
    ]
    for i, s in enumerate(stats):
        ax.text(0.05, 0.95 - i*0.11, s, transform=ax.transAxes, fontsize=8, fontfamily='monospace', verticalalignment='top')

    # ---- (3,1) |Bias| 分布直方图 ----
    ax = fig.add_subplot(gs[3, 1])
    ax.hist(abs_bias, bins=40, color='steelblue', edgecolor='white', alpha=0.8)
    ax.axvline(DEADBAND, color='orange', ls='--', linewidth=1.5, label=f'Deadband {DEADBAND}°')
    ax.axvline(DEADBAND_X2, color='red', ls=':', linewidth=1.5, label=f'Smoothstep end {DEADBAND_X2}°')
    ax.set_xlabel('|Y8_Bias| (°)'); ax.set_ylabel('Count'); ax.set_title('Bias Magnitude Distribution')
    ax.legend(fontsize=7); ax.grid(True, alpha=0.3)

    # 标题
    title = f'Y8 Line Tracking Diagnostic ({n} samples, {n*0.02:.1f}s)'
    if title_extra:
        title += f'\n{title_extra}'
    fig.suptitle(title, fontsize=13, fontweight='bold', y=0.99)

    out_path = os.path.splitext(path)[0] + '_analysis.png'
    fig.savefig(out_path, dpi=150, bbox_inches='tight', facecolor='white')
    print(f"Saved: {out_path}")
    plt.close(fig)

if __name__ == '__main__':
    main()
