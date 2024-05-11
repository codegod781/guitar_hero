import os
import re

if __name__ == "__main__":
    game_logic_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'colors.c')

    with open(game_logic_path, 'r') as f:
        lines = f.readlines()  # Read all lines from the file
        for i in range(len(lines)):
            if "palette[COLOR_COUNT]" in lines[i]:
                break  # Stop reading when you reach the line containing palette[COLOR_COUNT]
        
        palette_start_line = i
        print("case (pixel_data)")

        # Extract {R, G, B} values from each line
        for i in range(palette_start_line, len(lines)):
            # Use regex to find {R, G, B} values
            match = re.search(r'\[(\w+)\]\s*=\s*{(\d+), (\d+), (\d+)}', lines[i])
            if match:
                color_name = match.group(1)
                rgb_values = (int(match.group(2)), int(match.group(3)), int(match.group(4)))
                rgb_values = ['{:02x}'.format(x) for x in rgb_values]
                color_index = i - palette_start_line
                print(f"  6'd{color_index}: " + "{VGA_R, VGA_G, VGA_B} = " + f"24'h{rgb_values[0]}{rgb_values[1]}{rgb_values[2]}; // {color_name.capitalize()}")
        
        print("  default: {VGA_R, VGA_G, VGA_B} = 24'hffffff; // Default to white")
        print("endcase")