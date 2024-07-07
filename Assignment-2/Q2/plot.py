import matplotlib.pyplot as plt

execution_times = {}
with open("execution_times.txt", "r") as file:
    for line in file:
        fields = line.split()
        strategy, time = fields[0], float(fields[1])
        execution_times[strategy] = time

strategies = list(execution_times.keys())
times = [execution_times[strategy] for strategy in strategies]

plt.bar(strategies, times)
plt.xlabel("Scheduling Strategy")
plt.ylabel("Execution Time (seconds)")
plt.title("Execution Time Comparison")

plt.savefig("execution_time_comparison.png")

plt.show()

