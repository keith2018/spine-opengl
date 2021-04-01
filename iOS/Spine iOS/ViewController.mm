//
//  ViewController.m
//  Spine iOS
//
//  Created by keith2018 on 2021/3/29.
//

#import "ViewController.h"
#import "SpineController.h"

#define JSON_PATH   "Spine.bundle/boy/spineboy-ess.json"
#define ATLAS_PATH  "Spine.bundle/boy/spineboy.atlas"
#define SKIN_NAME   "default"
#define ANIM_NAME   "idle"
#define POS_X       120
#define POS_Y       800
#define SCALE       1.5


@interface ViewController ()

@end

@implementation ViewController
{
    GLKView *_glView;
    SpineController *_spineCtrl;
    
    CADisplayLink * _displayLink;
    CFAbsoluteTime _currTime;
}

- (void)viewDidLoad {
    [super viewDidLoad];
    
    _glView = [[GLKView alloc] initWithFrame:self.view.bounds context:[[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2]];
    [self.view addSubview:_glView];
    
    CGSize size = self.view.bounds.size;
    NSString *jsonPath = [[NSBundle mainBundle] pathForResource:@JSON_PATH ofType:nil];
    NSString *atlasPath = [[NSBundle mainBundle] pathForResource:@ATLAS_PATH ofType:nil];
    
    // create spine render
    [EAGLContext setCurrentContext:_glView.context];
    _spineCtrl = new SpineController(size.width, size.height);
    bool _createOk = _spineCtrl->spineCreate([atlasPath UTF8String], [jsonPath UTF8String], SKIN_NAME, POS_X, POS_Y, SCALE);
    if (!_createOk) {
        NSLog(@"load spine resources failed");
        return;
    }

    _createOk = _spineCtrl->spineSetAnimation(ANIM_NAME);
    if (!_createOk) {
        NSLog(@"set spine animation failed");
        return;
    }
    
    _currTime = 0;
    _displayLink = [CADisplayLink displayLinkWithTarget:self selector:@selector(render:)];
    [_displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
}

- (void)render:(CADisplayLink *)displayLink {
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    
    float dt = 0;
    if (_currTime > 0) {
        dt = CFAbsoluteTimeGetCurrent() - _currTime;
    }

    _spineCtrl->spineDraw(dt);
    _currTime = CFAbsoluteTimeGetCurrent();
    
    [_glView.context presentRenderbuffer:GL_RENDERBUFFER];
}


- (void)dealloc {
    [_displayLink invalidate];
    SAFE_DELETE(_spineCtrl);
}

@end
