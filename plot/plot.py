import matplotlib.pyplot as plt
import numpy as np

# Results from original implementation
original_data = {
    'spatial': {
        'hits': 5070,
        'misses': 4930,
        'total': 10000,
        'hit_rate': 50.7
    },
    'temporal': {
        'hits': 869,
        'misses': 9131,
        'total': 10000,
        'hit_rate': 8.69
    }
}

# Results from enhanced implementation
enhanced_data = {
    'spatial': {
        'l1_hits': 8637,
        'l1_misses': 1363,
        'l2_hits': 194,
        'l2_misses': 1166,
        'victim_hits': 1,
        'stream_hits': 2,
        'total_hits': 8637 + 194 + 1 + 2,
        'total': 10000,
        'hit_rate': 88.34
    },
    'temporal': {
        'l1_hits': 4646,
        'l1_misses': 5354,
        'l2_hits': 1386,
        'l2_misses': 3955,
        'victim_hits': 7,
        'stream_hits': 6,
        'total_hits': 4646 + 1386 + 7 + 6,
        'total': 10000,
        'hit_rate': 60.45
    }
}

# Create figure with multiple subplots
fig, axs = plt.subplots(2, 2, figsize=(18, 12))
fig.suptitle('Cache Performance Comparison: Original vs Enhanced Implementation', fontsize=16)

# 1. Hit Rate Comparison
patterns = ['Spatial', 'Temporal']
original_rates = [original_data['spatial']['hit_rate'], original_data['temporal']['hit_rate']]
enhanced_rates = [enhanced_data['spatial']['hit_rate'], enhanced_data['temporal']['hit_rate']]

x = np.arange(len(patterns))
width = 0.35

axs[0, 0].bar(x - width/2, original_rates, width, label='Original', color='lightblue')
axs[0, 0].bar(x + width/2, enhanced_rates, width, label='Enhanced', color='lightgreen')
axs[0, 0].set_ylabel('Hit Rate (%)')
axs[0, 0].set_title('Cache Hit Rate Comparison')
axs[0, 0].set_xticks(x)
axs[0, 0].set_xticklabels(patterns)
axs[0, 0].legend()
axs[0, 0].grid(axis='y', linestyle='--', alpha=0.7)

for i, v in enumerate(original_rates):
    axs[0, 0].text(i - width/2, v + 1, f"{v}%", ha='center', va='bottom')
for i, v in enumerate(enhanced_rates):
    axs[0, 0].text(i + width/2, v + 1, f"{v}%", ha='center', va='bottom')

# 2. Hit/Miss Distribution - Original
labels = ['Hits', 'Misses']
sizes_spatial = [original_data['spatial']['hits'], original_data['spatial']['misses']]
sizes_temporal = [original_data['temporal']['hits'], original_data['temporal']['misses']]

axs[0, 1].pie(sizes_spatial, labels=labels, autopct='%1.1f%%', 
           colors=['lightgreen', 'lightcoral'], startangle=90)
axs[0, 1].set_title('Original Implementation - Spatial Pattern')

# 3. Enhanced Implementation - Cache Level Contribution (Spatial)
labels = ['L1 Hits', 'L2 Hits', 'Victim Hits', 'Stream Hits', 'Misses']
sizes = [
    enhanced_data['spatial']['l1_hits'],
    enhanced_data['spatial']['l2_hits'],
    enhanced_data['spatial']['victim_hits'],
    enhanced_data['spatial']['stream_hits'],
    enhanced_data['spatial']['l2_misses']
]
explode = (0.1, 0, 0, 0, 0)  # explode L1 slice
colors = ['lightgreen', 'mediumseagreen', 'darkseagreen', 'palegreen', 'lightcoral']

axs[1, 0].pie(sizes, explode=explode, labels=labels, autopct='%1.1f%%',
           colors=colors, startangle=90)
axs[1, 0].set_title('Enhanced Implementation - Spatial Pattern')

# 4. Enhanced Implementation - Cache Level Contribution (Temporal)
sizes = [
    enhanced_data['temporal']['l1_hits'],
    enhanced_data['temporal']['l2_hits'],
    enhanced_data['temporal']['victim_hits'],
    enhanced_data['temporal']['stream_hits'],
    enhanced_data['temporal']['l2_misses']
]

axs[1, 1].pie(sizes, explode=explode, labels=labels, autopct='%1.1f%%',
           colors=colors, startangle=90)
axs[1, 1].set_title('Enhanced Implementation - Temporal Pattern')

# Adjust layout and save
plt.tight_layout(rect=[0, 0.03, 1, 0.95])
plt.savefig('cache_performance_comparison.png', dpi=300)
plt.show()

# Additional plot: Performance improvement by access pattern
plt.figure(figsize=(12, 6))
improvement_data = [
    enhanced_data['spatial']['hit_rate'] - original_data['spatial']['hit_rate'],
    enhanced_data['temporal']['hit_rate'] - original_data['temporal']['hit_rate']
]

plt.bar(patterns, improvement_data, color=['steelblue', 'darkseagreen'])
plt.title('Performance Improvement in Hit Rate (Enhanced vs Original)', fontsize=14)
plt.ylabel('Hit Rate Improvement (percentage points)')
plt.grid(axis='y', linestyle='--', alpha=0.7)

for i, v in enumerate(improvement_data):
    plt.text(i, v + 0.5, f"+{v:.2f}%", ha='center', va='bottom', fontweight='bold')

plt.tight_layout()
plt.savefig('hit_rate_improvement.png', dpi=300)
plt.show()