#!/vendor/bin/sh
echo "Long running IRQ metrics"
cat /sys/kernel/metrics/irq/long_irq_metrics
echo "Resume latency metrics"
cat /sys/kernel/metrics/resume_latency/resume_latency_metrics
echo "Temperature Residency Metrics:"
cat /sys/kernel/metrics/temp_residency/temp_residency_all/stats
