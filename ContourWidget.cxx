#include <vector>

#include <vtkSmartPointer.h>
#include <vtkProperty.h>
#include <vtkContourWidget.h>
#include <vtkDataSetMapper.h>
#include <vtkOrientedGlyphContourRepresentation.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkCommand.h>
#include <vtkDebugLeaks.h>
#include <vtkCamera.h>
#include <vtkPlane.h>
#include <vtkPolyData.h>
#include <vtkCellArray.h>
#include <vtkTriangle.h>
#include <vtkTriangleFilter.h>
#include <vtkPoints.h>
#include <vtkMath.h>
#include <vtkWidgetEvent.h>
#include <vtkWidgetEventTranslator.h>

vtkSmartPointer<vtkTriangleFilter> gen_triangles_as_polygons(vtkSmartPointer<vtkPolyData> cd)
{
    vtkSmartPointer<vtkPolyData> clsdPoly = vtkSmartPointer<vtkPolyData>::New();
    clsdPoly->Allocate();
    clsdPoly->SetPoints(cd->GetPoints());

    int numPts = cd->GetNumberOfPoints();
    vtkSmartPointer<vtkIdList> ids = vtkSmartPointer<vtkIdList>::New();
    ids->SetNumberOfIds(numPts);
    for(int i = 0; i != numPts; ++i)
    {
        ids->SetId(i, i);
    }

    clsdPoly->InsertNextCell(VTK_POLYGON, ids);

    vtkSmartPointer<vtkTriangleFilter> triangles = vtkSmartPointer<vtkTriangleFilter>::New();
    triangles->SetInput(clsdPoly);
    triangles->Update();
    
    return triangles;
}

vtkSmartPointer<vtkTriangleFilter> gen_triangles_as_lines(vtkSmartPointer<vtkPolyData> cd)
{
    vtkSmartPointer<vtkPolyData> clsdPoly = vtkSmartPointer<vtkPolyData>::New();
    clsdPoly->SetPoints(cd->GetPoints());
    clsdPoly->SetPolys(cd->GetLines());

    vtkSmartPointer<vtkTriangleFilter> triangles = vtkSmartPointer<vtkTriangleFilter>::New();
    triangles->SetInput(clsdPoly);
    triangles->Update();
    
    return triangles;
}

vtkSmartPointer<vtkTriangleFilter> gen_triangles(vtkSmartPointer<vtkPolyData> cd)
{
    return gen_triangles_as_polygons(cd);
}

class the_callback : public vtkCommand
{
    public: static the_callback *New()
    { 
        return new the_callback;
    }
    
    public: the_callback()
    {
    }
    
    public: virtual void Execute(vtkObject* w, unsigned long event, void *calldata)
    {
        if (event == vtkCommand::InteractionEvent)
        {
	        std::cout << "Interaction..." << std::endl;
	        
	        vtkContourWidget* widget = (vtkContourWidget*)w;
            vtkSmartPointer<vtkPolyData> cd = ((vtkOrientedGlyphContourRepresentation*)widget->GetRepresentation())->GetContourRepresentationAsPolyData();

            vtkSmartPointer<vtkTriangleFilter> td = gen_triangles(cd);
            _mapper->SetInputConnection(td->GetOutputPort());
	        
	        return;
        }
    }
    
    void set_mapper(vtkSmartPointer<vtkDataSetMapper> mapper) 
    {
        this->_mapper = mapper;
    }
    
    private: vtkDataSetMapper* _mapper;
};

vtkSmartPointer<vtkPolyData> make_contour_data(int n, double R)
{
    vtkSmartPointer<vtkPoints> points   = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkCellArray> lines = vtkSmartPointer<vtkCellArray>::New();

    vtkIdType* lineIndices = new vtkIdType[n+1];
    for (int k = 0; k != n; ++k)
    {
        double angle = 2.0 * vtkMath::Pi() * double(k)/20.0;
        points->InsertPoint(static_cast<vtkIdType>(k), R*cos(angle), R*sin(angle), 0.0 );
        lineIndices[k] = static_cast<vtkIdType>(k);    
    }
    lineIndices[n] = 0;
  
    lines->InsertNextCell(n+1, lineIndices);
    delete [] lineIndices;

    vtkSmartPointer<vtkPolyData> pd = vtkSmartPointer<vtkPolyData>::New();
  
    pd->SetPoints(points);
    pd->SetLines(lines);
    
    return pd;
}

int main( int argc, char *argv[] )
{
    // Create the RenderWindow, Renderer and both Actors
    vtkSmartPointer<vtkRenderer>     renderer = vtkSmartPointer<vtkRenderer>::New();
    renderer->SetBackground(0.1, 0.2, 0.4);

    vtkSmartPointer<vtkRenderWindow> window   = vtkSmartPointer<vtkRenderWindow>::New();
    window->SetSize(600, 600);
  
    window->AddRenderer(renderer);
    vtkSmartPointer<vtkRenderWindowInteractor> interactor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
    interactor->SetRenderWindow(window);

    vtkSmartPointer<vtkPolyData> cd = make_contour_data(20, 1.0);

    vtkSmartPointer<vtkOrientedGlyphContourRepresentation> contourRep = vtkSmartPointer<vtkOrientedGlyphContourRepresentation>::New();
    contourRep->GetLinesProperty()->SetColor(1, 0, 0); //set color to red
    contourRep->GetProperty()->SetColor(0, 1, 0); // color of something to green
    contourRep->GetActiveProperty()->SetColor(1, 1, 1); // color of something to green

    vtkSmartPointer<vtkContourWidget> contourWidget = vtkSmartPointer<vtkContourWidget>::New();
    contourWidget->SetInteractor(interactor);
    contourWidget->SetRepresentation(contourRep);
    contourWidget->On();

    contourWidget->Initialize(cd, 1);
    contourWidget->CloseLoop();
    cd = ((vtkOrientedGlyphContourRepresentation*)contourWidget->GetRepresentation())->GetContourRepresentationAsPolyData();
    
    vtkSmartPointer<vtkDataSetMapper> mapper = vtkSmartPointer<vtkDataSetMapper>::New();
    vtkSmartPointer<vtkTriangleFilter> td = gen_triangles(cd);
    mapper->SetInputConnection(td->GetOutputPort());
    
    vtkSmartPointer<the_callback> callback = vtkSmartPointer<the_callback>::New();
    callback->set_mapper(mapper);
    contourWidget->AddObserver(vtkCommand::InteractionEvent, callback);

    vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);
    actor->GetProperty()->SetColor(0,1,0);
    actor->GetProperty()->SetOpacity(0.2);

    renderer->AddActor(actor);
  
    contourWidget->Render();
    renderer->ResetCamera();
    window->Render();
    
    interactor->Initialize();
    interactor->Start();  
  
    contourWidget->Off();
  
    return EXIT_SUCCESS;
}

