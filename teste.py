from math import ceil
rows = 28
numBlocks = 4

blockSize = rows // numBlocks
remainder = rows % numBlocks

blocks = []

for _ in range(remainder):
    blocks.append(blockSize + 1)

for _ in range(remainder, numBlocks):
    blocks.append(blockSize)

print(blocks)
print(sum(blocks))


