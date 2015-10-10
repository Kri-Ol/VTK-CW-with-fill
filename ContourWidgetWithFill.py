#!/usr/bin/python 

import vtk
import math

def make_rwi():
    """
    makes renderer, window, interactor
    """
    renderer = vtk.vtkRenderer()
    renderer.SetBackground(0.1, 0.2, 0.4)
    
    window = vtk.vtkRenderWindow()
    window.SetSize(600, 600)
    
    window.AddRenderer(renderer)
    interactor = vtk.vtkRenderWindowInteractor()
    interactor.SetRenderWindow( window )    

    return (renderer, window, interactor)    
    
def gen_triangles_as_lines(cd):
    """
    from closed contour makes triangulated surface algorithm,
    using contour lines as base structure
    """
    clsdPoly = vtk.vtkPolyData()
    clsdPoly.SetPoints(cd.GetPoints())
    clsdPoly.SetPolys(cd.GetLines())

    triangles = vtk.vtkTriangleFilter()
    triangles.SetInput(clsdPoly)
    triangles.Update()
    
    return triangles
    
def gen_triangles_as_polygons(cd):
    """
    from closed contour makes triangulated surface algorithm,
    using polygons and points as base structure
    """
    clsdPoly = vtk.vtkPolyData()
    clsdPoly.Allocate()
    clsdPoly.SetPoints(cd.GetPoints())

    numPts = cd.GetNumberOfPoints()
    idList = vtk.vtkIdList()
    idList.SetNumberOfIds(numPts)
    [idList.SetId(i, i) for i in range(numPts)]

    clsdPoly.InsertNextCell(vtk.VTK_POLYGON, idList)

    triangles = vtk.vtkTriangleFilter()
    triangles.SetInput(clsdPoly)
    triangles.Update()
    
    return triangles

def gen_triangles(cd):
    """
    Algorithm selector
    """
    return gen_triangles_as_polygons(cd)

def make_contour_data(n, R):
    """
    Makes initial contour as a circle,
    with n points and R as radius
    """
    points = vtk.vtkPoints()
    lines  = vtk.vtkCellArray()
    
    for i in range(0, n):
        angle = 2.0 * math.pi * float(i) / float(n)
        points.InsertPoint(i, R * math.cos(angle), R * math.sin(angle), 0.0 )
        lines.InsertNextCell(i)

    lines.InsertNextCell(0)

    pd = vtk.vtkPolyData()
 
    pd.SetPoints(points)
    pd.SetLines(lines)
    
    return pd
    
def render_things(renderer, window, interactor):
    renderer.ResetCamera()
    window.Render()

    interactor.Initialize()
    interactor.Start()    
    
def write_poly(fname, pd):
    """
    Helper to save polydata into file as an XML
    """
    writer = vtk.vtkXMLPolyDataWriter()
    writer.SetInput(pd)
    writer.SetFileName(fname)
    writer.SetDataModeToAscii()
    writer.SetCompressorTypeToNone()
    writer.Write()
    
def my_callback(widget, event_string):
    """
    Callback to track contour changes and regenerate filled area
    """
    global mapper

    cd = widget.GetRepresentation().GetContourRepresentationAsPolyData()    

    td = gen_triangles(cd)
    mapper.SetInputConnection(td.GetOutputPort())

def main():
    global mapper

    renderer, window, interactor = make_rwi()

    cd = make_contour_data(20, 1.0)
    #write_poly("before.vtp", cd)

    contourRep = vtk.vtkOrientedGlyphContourRepresentation()
    contourRep.GetLinesProperty().SetColor(1, 0, 0) # set color to red
    contourRep.GetProperty().SetColor(0,1,0)
    contourRep.GetActiveProperty().SetColor(0,0,0)
 
    contourWidget = vtk.vtkContourWidget()
    contourWidget.SetInteractor(interactor)
    contourWidget.SetRepresentation(contourRep)
    contourWidget.On()

    contourWidget.Initialize(cd, 1)
    contourWidget.CloseLoop()
    cd = contourWidget.GetRepresentation().GetContourRepresentationAsPolyData()
    #write_poly("after.vtp", cd)
    
    contourWidget.AddObserver('InteractionEvent', my_callback)
    contourWidget.Render()
    
    mapper = vtk.vtkDataSetMapper()
    td = gen_triangles(cd)
    mapper.SetInputConnection(td.GetOutputPort())

    actor = vtk.vtkActor()
    actor.SetMapper(mapper)
    actor.GetProperty().SetColor(0,1,0)
    actor.GetProperty().SetOpacity(0.2)  #actor.GetProperty().SetRepresentationToWireframe()

    renderer.AddActor(actor)

    render_things(renderer, window, interactor)
    
if __name__ == "__main__":
    main()

