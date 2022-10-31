
*** DESIGN DOCUMENT FOR MPLC ***
Target: ability to create a Figure as new Figure(fig_siz_px) and then subsequently call
fig.scatter(...) and/or fig.lines(...). The axes should auto-update and all data should be 
rendered in a way mimicking matplotlib.
Accessors set_xlim etc would be accessible. Similar to previously, a FigureRenderObj could
be responsible for rendering all elements.

The Canvas2D object is the base class for scatter, lines, histogram etc, and holds both 
the data setters for the respective type, and also the OpenGL VAO pointers used in 
Figure->render()



