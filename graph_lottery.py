#!/usr/bin/env python3
"""
Script to generate a graph showing lottery scheduler behavior.
Usage: python3 graph_lottery.py < lottery_data.csv
"""

import sys
import csv

def main():
    # Read CSV data from stdin
    reader = csv.DictReader(sys.stdin)
    
    samples = []
    process_a = []
    process_b = []
    process_c = []
    
    for row in reader:
        samples.append(int(row['sample']))
        process_a.append(int(row['process_a']))
        process_b.append(int(row['process_b']))
        process_c.append(int(row['process_c']))
    
    # Try to use matplotlib if available
    try:
        import matplotlib.pyplot as plt
        
        plt.figure(figsize=(12, 6))
        plt.plot(samples, process_a, 'b-', label='Process A (30 tickets)', linewidth=2)
        plt.plot(samples, process_b, 'g-', label='Process B (20 tickets)', linewidth=2)
        plt.plot(samples, process_c, 'r-', label='Process C (10 tickets)', linewidth=2)
        
        plt.xlabel('Sample Number', fontsize=12)
        plt.ylabel('Time Slices Allocated', fontsize=12)
        plt.title('xv6 Lottery Scheduler - Time Slice Allocation (3:2:1 Ticket Ratio)', fontsize=14)
        plt.legend(fontsize=10)
        plt.grid(True, alpha=0.3)
        
        # Add expected ratio line (for reference)
        total_tickets = 60
        expected_a = (30.0/total_tickets) * sum(process_a + process_b + process_c) / len(samples)
        expected_b = (20.0/total_tickets) * sum(process_a + process_b + process_c) / len(samples)
        expected_c = (10.0/total_tickets) * sum(process_a + process_b + process_c) / len(samples)
        
        plt.axhline(y=expected_a, color='b', linestyle='--', alpha=0.3, label=f'Expected A: {expected_a:.1f}')
        plt.axhline(y=expected_b, color='g', linestyle='--', alpha=0.3, label=f'Expected B: {expected_b:.1f}')
        plt.axhline(y=expected_c, color='r', linestyle='--', alpha=0.3, label=f'Expected C: {expected_c:.1f}')
        
        plt.tight_layout()
        plt.savefig('lottery_scheduler_graph.png', dpi=300, bbox_inches='tight')
        print("Graph saved as 'lottery_scheduler_graph.png'", file=sys.stderr)
        plt.show()
        
    except ImportError:
        # Fallback: print ASCII art graph
        print("\nmatplotlib not available. Printing summary statistics:\n")
        
        total_a = sum(process_a)
        total_b = sum(process_b)
        total_c = sum(process_c)
        total = total_a + total_b + total_c
        
        if total > 0:
            print(f"Total time slices:")
            print(f"  Process A (30 tickets): {total_a} ({100*total_a/total:.1f}%)")
            print(f"  Process B (20 tickets): {total_b} ({100*total_b/total:.1f}%)")
            print(f"  Process C (10 tickets): {total_c} ({100*total_c/total:.1f}%)")
            print(f"\nExpected distribution: A=50%, B=33.3%, C=16.7%")
            print(f"Actual ratio: {total_a/total_c:.2f}:{total_b/total_c:.2f}:1")
            print(f"Expected ratio: 3:2:1")
        
        # Simple ASCII bar chart
        print("\nASCII Bar Chart (normalized):")
        max_val = max(total_a, total_b, total_c)
        scale = 50.0 / max_val if max_val > 0 else 1
        
        print(f"Process A: {'=' * int(total_a * scale)} ({total_a})")
        print(f"Process B: {'=' * int(total_b * scale)} ({total_b})")
        print(f"Process C: {'=' * int(total_c * scale)} ({total_c})")

if __name__ == '__main__':
    main()
