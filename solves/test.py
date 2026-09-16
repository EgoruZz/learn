x, y, z = map(int, input().split())
print("Impossible" if (x + y < z) else x + y - z)