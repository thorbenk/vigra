import vigra
import vigra.graphs as vigraph
import pylab
import numpy
np=numpy
import sys
import matplotlib
import pylab as plt
import math
from matplotlib.widgets import Slider, Button, RadioButtons









print "get input"
f       = '100075.jpg'
f       = '69015.jpg'
#f       = "/media/tbeier/GSP1RMCPRFR/iso.03530.png"
img     = vigra.impex.readImage(f)

if(img.shape[2]==1):
    print "lab",img.shape 
    img = numpy.concatenate([img]*3,axis=2)
    #img = numpy.swapaxes(img,0,2)
    #img = numpy.swapaxes(img,0,1)
    imgLab = img
    print "lab",imgLab.shape
    imgLab = vigra.taggedView(imgLab,'xyc')
else:
    imgLab = vigra.colors.transform_RGB2Lab(img)
sigma   = 1.0


imgLab-=imgLab.min()
imgLab/=imgLab.max()
imgLab*=255

img-=img.min()
img/=img.max()
img*=255

print imgLab.shape


print "interpolate image"
imgLabBig = vigra.resize(imgLab,[imgLab.shape[0]*2+1,imgLab.shape[1]*2+1 ])


#gradmag = numpy.squeeze(vigra.filters.gaussianGradientMagnitude(imgLab,sigma))
gradmag = numpy.squeeze(vigra.filters.gaussianGradientMagnitude(imgLabBig,sigma))
hessian = numpy.squeeze(vigra.filters.hessianOfGaussianEigenvalues(imgLabBig[:,:,0],sigma))[:,:,0]
hessian-=hessian.min()
raw     = 256-imgLabBig[:,:,0].copy()
#vigra.imshow(hessian)
#vigra.show()

gridGraph  = vigraph.gridGraph(imgLab.shape[:2],False)  

def makeWeights(gamma):
    global hessian,gradmag,gridGraph
    print "hessian",hessian.min(),hessian.max()
    print "raw ",raw.min(),raw.max()
    wImg= numpy.exp((gradmag**0.5)*gamma*-1.0)#**0.5
    wImg = numpy.array(wImg).astype(numpy.float32)

    w=vigra.graphs.edgeWeightsFromIterpolatedImage(gridGraph,wImg)

    return w

weights  = makeWeights(3.0)


pathFinder = vigraph.ShortestPathPathDijkstra(gridGraph)


visuimg =img.copy()
ax = plt.gca()
fig = plt.gcf()
visuimg-=visuimg.min()
visuimg/=visuimg.max()
implot = ax.imshow(numpy.swapaxes(visuimg,0,1),cmap='gray')

clickList=[]


frozen = False




def makeVisuImage(path,img):
    coords = (path[:,0],path[:,1])
    visuimg =img.copy()
    iR=visuimg[:,:,0]
    iG=visuimg[:,:,1]
    iB=visuimg[:,:,2]
    iR[coords]=255
    iG[coords]=0
    iB[coords]=0
    visuimg-=visuimg.min()
    visuimg/=visuimg.max()  
    return visuimg


def onclick(event):
    global clickList
    global weights
    global img
    if event.xdata != None and event.ydata != None:


        xRaw,yRaw = event.xdata,event.ydata
        

        
        if not frozen and xRaw >=0.0 and yRaw>=0.0 and xRaw<img.shape[0] and yRaw<img.shape[1]:
            x,y = long(math.floor(event.xdata)),long(math.floor(event.ydata))
            print "inside click",x,y
            clickList.append((x,y))
            if len(clickList)==1:
                print "source ",clickList[0]
            elif len(clickList)==2:
                print "target ",clickList[1]

                print "run path finder"

                source = gridGraph.coordinateToNode(clickList[0])
                target = gridGraph.coordinateToNode(clickList[1])
                weights  = makeWeights(sgamma.val)
                path = pathFinder.run(weights, source,target,weightType='edgeWeights').path(pathType='coordinates')
                visuimg = makeVisuImage(path,img)
                implot.set_data(numpy.swapaxes(visuimg,0,1))
                plt.draw()

        else:
            print "outside click"
            

axslider = plt.axes([0.0, 0.00, 0.4, 0.075])
axfreeze   = plt.axes([0.6, 0.00, 0.1, 0.075])
axunfreeze   = plt.axes([0.8, 0.00, 0.1, 0.075])

bfreeze   = Button(axfreeze, 'freeze')
bunfreeze = Button(axunfreeze, 'unfrease and clear')

sgamma = Slider(axslider, 'gamma', 0.01, 5.0, valinit=1.0)


def freeze(event):
    global frozen
    frozen=True

def unfreeze(event):
    global frozen,clickList
    frozen=False
    clickList = []

def onslide(event):
    global img,gradmag,weights,clickList,sgamma
    weights[:]  = makeWeights(sgamma.val)
    print "onslide",clickList
    if len(clickList)>=2:
        print "we have  path"
        source = gridGraph.coordinateToNode(clickList[0])
        target = gridGraph.coordinateToNode(clickList[1])
        path = pathFinder.run(weights, source,target).path(pathType='coordinates')

        


        visuimg = makeVisuImage(path,img)
        implot.set_data(numpy.swapaxes(visuimg,0,1))
        plt.draw()


bfreeze.on_clicked(freeze)
bunfreeze.on_clicked(unfreeze)
sgamma.on_changed(onslide)

cid = fig.canvas.mpl_connect('button_press_event', onclick)
plt.show()
