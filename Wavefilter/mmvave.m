data = load('haomibo.mat');  % 假设您的数据存储在变量名haomibo中
data = struct2array(data);

order = 3000;
cutoff_frequency = 0.005;
fir_filter = fir1(order, cutoff_frequency);

% 第一次滤波操作
background_signal_1 = filter(fir_filter, 1, data);
background_signal_1(1:20000) = data(1:20000);  % 保留前20000个数据点
feature_pulse_signal_1 = data - background_signal_1;
feature_pulse_signal_1(1:20000) = 0;  % 前20000个数据点设为0

% 将第一次得到的特征脉冲信号作为第二次的输入
% 第二次滤波操作
background_signal_2 = filter(fir_filter, 1, feature_pulse_signal_1);
background_signal_2(1:20000) = 0;  % 前20000个数据点设为0
feature_pulse_signal_2 = feature_pulse_signal_1 - background_signal_2;
feature_pulse_signal_2(1:20000) = 0;  % 前20000个数据点设为0

% 更新背景信号，将第二次的背景信号加到第一次的背景信号上
background_signal = background_signal_1 + background_signal_2;

% 找到第一次特征脉冲信号的峰值
[peaks, locations] = findpeaks(feature_pulse_signal_1);

% 计算峰值的平均值
threshold = mean(peaks);

% 使用阈值来识别第二次特征脉冲信号
feature_pulse_signal_2 = feature_pulse_signal_1;
feature_pulse_signal_2(feature_pulse_signal_2 < threshold) = 0;

% 剩下的就是第二次背景信号
background_signal_2 = feature_pulse_signal_1 - feature_pulse_signal_2;

% 更新背景信号
background_signal = background_signal_1 + background_signal_2;

% 绘图
figure;
subplot(4,1,1);
plot(data);
title('原始信号');

subplot(4,1,2);
plot(background_signal);
title('背景信号'); 

subplot(4,1,3);
plot(feature_pulse_signal_1, 'b');  % 使用蓝色绘制第一次特征脉冲信号
title('第一次特征脉冲信号');

subplot(4,1,4);
plot(feature_pulse_signal_2, 'r');  % 使用红色绘制第二次特征脉冲信号
title('第二次特征脉冲信号（基于阈值）');

% 平滑第二次特征信号 - 使用多项式拟合
x = (1:length(feature_pulse_signal_2))';
p = polyfit(x, feature_pulse_signal_2, 10);  % 10阶多项式拟合
smoothed_feature_pulse_signal_2_poly = polyval(p, x);

% 平滑第二次特征信号 - 使用滑动平均
windowSize = 20;  % 定义窗口大小
smoothed_feature_pulse_signal_2_movmean = movmean(feature_pulse_signal_2, windowSize);

% 计算滑动平均的均值
mean_value = mean(smoothed_feature_pulse_signal_2_movmean);

% 从滑动平均结果中减去均值，以得到真正的特征信号
true_feature_signal = smoothed_feature_pulse_signal_2_movmean - mean_value;

% 将所有负值和零的元素设置为零
true_feature_signal(true_feature_signal <= 0) = 0;

% 初始化条件，以确保循环至少执行一次
mean_value_after_220000 = inf;

% 循环，直到满足停止条件
while mean_value_after_220000 >= 1e-4
    % 计算整个信号的均值
    mean_value = mean(true_feature_signal);
    
    % 减去均值
    true_feature_signal = true_feature_signal - mean_value;
    
    % 保留正值
    true_feature_signal(true_feature_signal <= 0) = 0;
    
    % 计算第22万个数据点之后的均值
    mean_value_after_220000 = mean(true_feature_signal(220001:end));
end

% 绘图
figure;
subplot(3,1,1);
plot(feature_pulse_signal_2, 'r');  % 使用红色绘制原始第二次特征脉冲信号
title('原始第二次特征脉冲信号');

subplot(3,1,2);
plot(smoothed_feature_pulse_signal_2_poly, 'b');  % 使用蓝色绘制多项式拟合的平滑信号
title('多项式拟合的平滑信号');

subplot(3,1,3);
plot(smoothed_feature_pulse_signal_2_movmean, 'g');  % 使用绿色绘制滑动平均的平滑信号
title('滑动平均的平滑信号');

figure;  % 创建一个新的图形窗口
plot(true_feature_signal);  % 绘制true_feature_signal
title('True Feature Signal');  % 设置图标题
grid on;  % 显示网格
