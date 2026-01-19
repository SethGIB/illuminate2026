import java.util.List;
//import java.awt.event.MouseEvent;

class VPoint
{
  PVector pos;
  
  VPoint(float _px, float _py)
  {
    this.pos = new PVector(_px, _py);    
  }
  
  void step(float _px, float _py)
  {
    this.pos = new PVector(_px, _py);
  }
  
  void show()
  {
    circle(this.pos.x,this.pos.y,4);
    circle(this.pos.x,this.pos.y,20);
  }
}

class VoronoiMgr
{
  VoronoiMgr()
  {
    this.mVoronoi = new Voronoi();
    this.mVPoints = new ArrayList();
  }

  private Voronoi mVoronoi;
  private ArrayList<VPoint> mVPoints;
  
  private boolean mIsDragging = false;
  private VPoint mDragPoint = null; 
  
  public void addSite(float sx, float sy)
  {
    this.mVPoints.add(new VPoint(sx,sy));    
    this.mVoronoi.addPoint(new Vec2D(sx, sy));
  }
  
  public void setupRandom(int count, int xMin, int xMax, int yMin, int yMax, boolean reset)
  {
    if(reset)
    {
      this.mVPoints.clear();
      this.mVoronoi = new Voronoi();
    }
    for(int p=0;p<count;p++)
    {
      float px = random(xMin,xMax);
      float py = random(yMin,yMax);
      this.addSite(px,py);
    }
  }
  
  public void setupGrid(int xDim, int yDim, int xBound, int yBound, boolean reset)
  {
    if(reset)
    {
      this.mVPoints.clear();
      this.mVoronoi = new Voronoi();
    }
    for(int y=0;y<yDim;y++)
    {
      for(int x=0;x<xDim;x++)
      {
        float px = map(x+0.5f,0,xDim,0,xBound);
        float py = map(y+0.5f,0,yDim,0,yBound);
        this.addSite(px,py);
      }
    }    
  }
 
  public void setupRadials(int count, PVector orig, PVector res, PVector rad, boolean reset)
  {
    if(reset)
    {
      this.mVPoints.clear();
      this.mVoronoi = new Voronoi();
    }

    
    for(int r=0;r<count;r++)
    {
      float rad0 = map(r,0,count,rad.x,rad.y);
      float res0 = map(r,0,count,res.x,res.y);
      radial((int)orig.x,(int)orig.y,(int)res0,(int)rad0);
    }
  }
  
  private void radial(int cx, int cy, int res, int rad)
  {
    for(int i=0;i<res;i++)
    {
      float th = map(i,0,res,-PI,PI);
      float rx = (rad * cos(th)) + cx;
      float ry = (rad * sin(th)) + cy;
 
      this.addSite(rx,ry);
    }
  }
  
  public void update()
  {
    this.mVoronoi = new Voronoi();
    if(this.mVPoints.size() > 0 )
    {
      for(var vp : this.mVPoints )
      {
        this.mVoronoi.addPoint(new Vec2D(vp.pos.x,vp.pos.y));
      }
    }
  }
  
  public void reset()
  {
  }
  
  public void showPoints()
  {
    for(var vp : this.mVPoints)
    {
      circle(vp.pos.x, vp.pos.y, 4);
      circle(vp.pos.x, vp.pos.y, 20);
    }
  }
  
  public void showVoronoi()
  {
    
  }
  
  public List<Polygon2D> regions()
  {
    return this.mVoronoi.getRegions();
  }
  
  public List<Triangle2D> delaunay()
  {
    return this.mVoronoi.getTriangles();
  }
  
  public void onMousePressed(MouseEvent e)
  {
    var btn = e.getButton();
    int id = -1;
    PVector eventPos = new PVector(e.getX(),e.getY());
    
    boolean clickedSite = false;
    for(VPoint vp : this.mVPoints)
    {
      id +=1;
      if(vp.pos.dist(eventPos) < 10 )
      {
        clickedSite = true;
        break;
      }
    }
    
    if(btn==37)
    {
      if(clickedSite)
      {
        this.mIsDragging = true;
        this.mDragPoint = this.mVPoints.get(id);
        return;
      }
      else
      {
        this.addSite(eventPos.x, eventPos.y);
        this.update();
        return;
      }
    }
    else if(btn==39)
    {
      this.mVPoints.remove(id);
      this.update();
      return;
    }
  }
  
  public void onMouseDragged(MouseEvent e)
  {
    if(this.mIsDragging && this.mDragPoint!=null)
    {
      this.mDragPoint.pos.set(mouseX, mouseY);
    }
  }
  
  public void onMouseReleased()
  {
    if(this.mIsDragging)
    {
      this.mIsDragging = false;
      this.mDragPoint = null;
      this.update();
    }
  }
}
