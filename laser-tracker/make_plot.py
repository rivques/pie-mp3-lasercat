from main import px_to_laser_polar
import matplotlib.pyplot as plt
import numpy as np

# side-by-side filled contour plot of distance and angle against x and y coordinates
def make_plot():
    # create a grid of x and y coordinates
    x = np.linspace(0, 640, 100)
    y = np.linspace(0, 480, 100)
    X, Y = np.meshgrid(x, y)

    # calculate distance and angle for each point in the grid
    distance = np.zeros_like(X)
    angle = np.zeros_like(X)
    for i in range(X.shape[0]):
        for j in range(X.shape[1]):
            angle[i, j], distance[i, j] = px_to_laser_polar(X[i, j], Y[i, j])

    # create a figure with two subplots
    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(12, 6))

    # large font size for titles and labels
    plt.rcParams.update({'font.size': 20})

    # plot distance
    contour1 = ax1.contourf(X, Y, distance, levels=50, cmap='viridis')
    ax1.set_title('Distance for a given pixel')
    ax1.set_xlabel('X (pixels)', fontsize=18)
    ax1.set_ylabel('Y (pixels)', fontsize=18)
    ax1.invert_yaxis()  # invert y-axis to match image coordinates
    ax1.set_aspect('equal', adjustable='box')  # set equal aspect ratio
    # large font size for tick labels
    ax1.tick_params(axis='both', which='major', labelsize=16)
    

    # unit label for colorbar
    cbar1 = fig.colorbar(contour1, ax=ax1)
    cbar1.set_label('Distance (mm)')

    # plot angle
    contour2 = ax2.contourf(X, Y, angle, levels=50, cmap='PiYG')
    ax2.set_title('Heading for a given pixel')
    ax2.set_xlabel('X (pixels)', fontsize=18)
    ax2.set_ylabel('Y (pixels)', fontsize=18)
    ax2.invert_yaxis()  # invert y-axis to match image coordinates
    ax2.set_aspect('equal', adjustable='box')  # set equal aspect ratio
    # large font size for tick labels
    ax2.tick_params(axis='both', which='major', labelsize=16)
    # unit label for colorbar
    cbar2 = fig.colorbar(contour2, ax=ax2)
    cbar2.set_label('Angle (degrees)')

    plt.tight_layout(pad=0)
    plt.show()

if __name__ == "__main__":
    make_plot()