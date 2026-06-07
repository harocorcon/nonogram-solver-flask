from flask import Flask, render_template

app = Flask(__name__)
UNKNOWN = 0
FILLED = 1
CROSSED = -1

def load_puzzle():
    with open("puzzle.txt", "r") as f:
        lines = [line.strip() for line in f.readlines()]
    
    dimensions = lines[0].split()
    rows = int(dimensions[0])
    cols = int(dimensions[1])

    row_clues = []
    col_clues = []
    if len(lines) < rows + cols + 1:
        print("Invalid puzzle format")
        raise ValueError("Invalid puzzle format: number of lines does not match expected dimensions.")
    
    board = []
    for idx, line in enumerate(lines[1:], start=0):
        clues = list(map(int, line.split()))
        if idx < rows:
            row_clues.append(clues)
        elif idx < rows + cols:
            col_clues.append(clues)
        else:
            board.append([int(x) for x in line.split()])
    

    # for r in range(rows):
    #     row = []
    #     for c in range(cols):
    #         row.append(0)
        
    #     board.append(row)

    board[5][2] = CROSSED;  
    return board, row_clues, col_clues



@app.route("/")

def home():

    board, row_clues, col_clues = load_puzzle()

    return render_template(
        "index.html",
        board=board,
        row_clues=row_clues,
        col_clues=col_clues
    )

if __name__ == "__main__":
    app.run(debug=True)