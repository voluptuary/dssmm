"""SuperFW's board-specific flash image budgets, matching upstream MAXFSIZE."""
LIMITS = {'sd':512*1024,'lite':496*1024,'chis':2048*1024}
def check_size(board,size):
    if board not in LIMITS:raise ValueError(f'Unknown board: {board}')
    limit=LIMITS[board]
    if size<=0 or size>limit:raise ValueError(f'{board}: image {size} bytes exceeds/invalid for {limit}-byte budget')
    return {'firmware_bytes':size,'firmware_limit_bytes':limit,'firmware_headroom_bytes':limit-size}
