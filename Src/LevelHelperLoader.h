//  This file was generated by LevelHelper
//  http://www.levelhelper.org
//
//  LevelHelperLoader.h
//  Created by Bogdan Vladu
//  Copyright 2011 Bogdan Vladu. All rights reserved.
//
////////////////////////////////////////////////////////////////////////////////
//
//  This software is provided 'as-is', without any express or implied
//  warranty.  In no event will the authors be held liable for any damages
//  arising from the use of this software.
//  Permission is granted to anyone to use this software for any purpose,
//  including commercial applications, and to alter it and redistribute it
//  freely, subject to the following restrictions:
//  The origin of this software must not be misrepresented; you must not
//  claim that you wrote the original software. If you use this software
//  in a product, an acknowledgment in the product documentation would be
//  appreciated but is not required.
//  Altered source versions must be plainly marked as such, and must not be
//  misrepresented as being the original software.
//  This notice may not be removed or altered from any source distribution.
//  By "software" the author refers to this code file and not the application 
//  that was used to generate this file.
//
////////////////////////////////////////////////////////////////////////////////
//
//  Version history
//  ...............
//  v3.8 Added moveSpriteWithUniqueName onPathWithUniqueName - made header nice
//  v3.9 Added uniqueName on bezier bodies
//  v4.0 Fix a release issue
//  v4.1 Physic shapes now doent convert so simulation behaves the same on all devices
//  v4.2 Proprietary implementation for parallax continuous scrolling (better)
//  v4.3 Added repeat next/prev frame methods
////////////////////////////////////////////////////////////////////////////////

#import <Foundation/Foundation.h>
#import "cocos2d.h"
#import "Box2D.h"

enum LevelHelper_TAG 
{ 
	DEFAULT_TAG 	= 0,
	STAR_1 			= 1,
	STAR_2 			= 2,
	STAR_3 			= 3,
	FLEAK 			= 4,
	NUMBER_OF_TAGS 	= 5
};

struct LHJoint {
    int tag;
    void* userData; //put your custom data here
};
typedef struct LHJoint LHJoint;

#define BATCH_NODE_CAPACITY 100 //you should change this value if you have more then 100 sprites in a texture image

//Pixel to metres ratio. Box2D uses metres as the unit for measurement.
//This ratio defines how many pixels correspond to 1 Box2D "metre"
//Box2D is optimized for objects of 1x1 metre therefore it makes sense
//to define the ratio so that your most common object type is 1x1 metre.
#define PTM_RATIO 32 //you should only keep this here

@protocol LevelHelperLoaderCustomCCSprite
@optional

-(CCSprite*) spriteFromDictionary:(NSDictionary*)spriteProp;

-(CCSprite*) spriteWithBatchFromDictionary:(NSDictionary*)spriteProp 
								   batchNode:(CCSpriteBatchNode*)batch;

-(void) removeFromBatchNode:(CCSprite*)sprite;


//this two methods should be overloaded together
//first one is for physic sprites 
-(void) setCustomAttributesForPhysics:(NSDictionary*)spriteProp 
                       body:(b2Body*)body
                     sprite:(CCSprite*)sprite;
//this second one is for the case where you dont use physics or you have sprites
//with "NO PHYSIC" as physic type
-(void) setCustomAttributesForNonPhysics:(NSDictionary*)spriteProp 
                               sprite:(CCSprite*)sprite;


@end

@interface LevelHelperLoader : NSObject<LevelHelperLoaderCustomCCSprite> {
	
	NSArray* lhSprites;	//array of NSDictionary with keys GeneralProperties (NSDictionary) 
						//and PhysicProperties (NSDictionary)
	NSArray* lhJoints;	//array of NSDictionary
    NSArray* lhParallax;//array of NSDictionary 
    NSArray* lhBeziers; //array of NSDictionary
    NSArray* lhAnims;   //array of NSDictionary

	NSMutableDictionary* ccSpritesInScene;	//key - uniqueSpriteName	value - CCSprite* or NSValue with b2Body*
	NSMutableDictionary* noPhysicSprites;   //key - uniqueSpriteName    value - CCSprite*
	NSMutableDictionary* ccJointsInScene;   //key - uniqueJointName     value - NSValue withPointer of b2Joint*
    NSMutableDictionary* ccParallaxInScene; //key - uniqueParallaxName  value - CCParallaxNode*
    NSMutableDictionary* ccBeziersBodyInScene;  //key - uniqueBezierName    value - NSValue withPointer of b2Body*
    NSMutableDictionary* ccBezierPathsInScene; //key - uniqueBezierName value - NSArray with NSValue with Point;
	NSMutableDictionary* batchNodes;		//key - textureName			value - NSDictionary
    NSMutableDictionary* ccFramesForAnimation;//key uniqueAnimName      value - NSMutableArray of CCSpriteFrame
	NSDictionary* wb; //world boundaries Info
    b2Body* wbBody;
    
	bool addSpritesToLayerWasUsed;
	bool addObjectsToWordWasUsed;
    
    bool convertPhysic;
    CGPoint safeFrame;
    CGPoint gravity;
	
    id pathNotifierId;
    SEL pathNotifierSel;
    
    id animNotifierId;
    SEL animNotifierSel;
    bool notifOnLoopForeverAnim;
    
	CCLayer* cocosLayer; //hold pointer to properly release the sprites
    
    bool convertLevel;
    CGPoint convertRatio;
}

////////////////////////////////////////////////////////////////////////////////
//                          INITIALIZING METHODS
////////////////////////////////////////////////////////////////////////////////
-(id) initWithContentOfFile:(NSString*)levelFile;

-(id) initWithContentOfFileFromInternet:(NSString*)webAddress;

-(id) initWithContentOfFile:(NSString*)levelFile 
			 levelSubfolder:(NSString*)levelFolder;

//END                        INITIALIZING METHODS
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
//                          CONSTRUCT WORLD METHODS
////////////////////////////////////////////////////////////////////////////////
/*
 //DISCUSSION
 this will create all the b2Body with the attached sprites.
 PLEASE READ THIS - in order to prepare this class for v1.4 I have added in the userData of CCSprite the following NSMutableDictionary
 this is also necessary in order to make follow path work with static physic sprites or else the position will be reset by the physic update
 
 so 
 b2Body     userData -> CCSprite
 CCSprite   userData -> NSMutableDictionary ->  key "UniqueName" returns NSString
 key "AnimName" returns NSString
 key "CurrentFrame" returns NSNumber numberWithInt in order to know at what frame in the animation you are (only updates on nextFrameForSpriteWithUniqueName and prevFrameForSpriteWithUniqueName)
 key "Body" returns NSValue -> use: b2Body* body = (b2Body)[value pointerValue];
 
 */
-(void) addObjectsToWorld:(b2World*)world cocos2dLayer:(CCLayer*)cocosLayer;

-(void) addSpritesToLayer:(CCLayer*)cocosLayer; //NO PHYSICS
//END                      CONSTRUCT WORLD METHODS
////////////////////////////////////////////////////////////////////////////////

/*
 DISCUSSION:
 Please call this method just before box2d update (the for loop from the tick function)
 NEEDED FOR
 - parallax scrolling
 - follow path to work correctly on sprites with static/kynematic body
 */
-(void) update:(ccTime) dt;


////////////////////////////////////////////////////////////////////////////////
//                          HELPER FUNCTIONS
////////////////////////////////////////////////////////////////////////////////
//DISCUSSION - all this methods will fail if you change the userData of a CCSprite*
//
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//this method will return nil if fail
+(NSString*) uniqueNameForSprite:(CCSprite*)sprite;
//this method will return nil if fail
+(NSString*) uniqueNameForBody:(b2Body*)body;
//------------------------------------------------------------------------------
//if fail will return -1
+(int) tagForSprite:(CCSprite*)sprite;
//if fail will return -1
+(int) tagForBody:(b2Body*)body;
//------------------------------------------------------------------------------
+(b2Body*) bodyForSprite:(CCSprite*)sprite;
//if fail will return nil
+(CCSprite*) spriteForBody:(b2Body*)body;
//------------------------------------------------------------------------------
//this method will return nil if fail
+(NSString*) animationNameOnBody:(b2Body*)body;
//this method will return nil if fail
+(NSString*) animationNameOnSprite:(CCSprite*)sprite;
//------------------------------------------------------------------------------
//this method will return -1 if fail
+(int) currentFrameOnSprite:(CCSprite*)sprite;
//this method will return -1 if fail
+(int) currentFrameOnBody:(b2Body*)body;
//------------------------------------------------------------------------------
//this method will return -1 if fail
+(int) tagForJoint:(b2Joint*)joint;
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//END                        HELPER FUNCTIONS
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
//                          CONVERSION METHODS
////////////////////////////////////////////////////////////////////////////////
//you will have to call this before initWithContentOfFile if you dont want retina
//[LevelHelperLoader useRetinaOnIpad:false];
+(void) useRetinaOnIpad:(bool)value;

//this will convert level to the used devide - safe frame must be the size of the
//device resolution inside LevelHelper - by default convertLevel is TRUE
//only call this method do deactivate the convert level option
-(void) convertLevel:(bool)value;

//in order to make physic behave the same on all devices physic shapes are not
//converted. Use this if you want to convert them pass true right after initWithContentOfFile
-(void) convertPhysicShapes:(bool)value;

//use this to get the correct converted position
//gives the correct point in all situations
-(CGPoint) convertedPoint:(CGPoint)pt;

//END                       CONVERSION METHODS
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
//                   RETRIEVAL  METHODS
////////////////////////////////////////////////////////////////////////////////
-(CCSprite*) spriteWithUniqueName:(NSString*)name; 
-(b2Body*) bodyWithUniqueName:(NSString*)name;
//------------------------------------------------------------------------------
//example of use
/*
 NSArray* arr = [kh spritesWithTag:tag];
 for(CCSprite* spr in arr){ ... }
*/
-(NSArray*) spritesWithTag:(LevelHelper_TAG)tag;
//example of use
/*
NSArray* arr = [lh bodiesWithTag:YOUR TAG];
 for(NSValue* val in arr)
 {
    b2Body* body = [val pointerValue];
    ...
 }
*/
-(NSArray*) bodiesWithTag:(LevelHelper_TAG)tag;
//------------------------------------------------------------------------------
-(b2Joint*) jointWithUniqueName:(NSString*)name;

/*
 returns NSMutableArray containing NSValue valueWithPoints:b2Joint
 //Example of use
 for(NSValue* val in arrayReturned)
 {
 b2Joint* joint = (b2Joint*)[val pointerValue];
 }
 */
-(NSArray*) jointsWithTag:(LevelHelper_TAG)tag;
//------------------------------------------------------------------------------
-(CCSpriteBatchNode*) batchNodeForFile:(NSString*) imageName;
-(NSArray*) batchNodesInLevel;
//------------------------------------------------------------------------------
//this method will return nil for parallaxes with continuous scrolling.
//continuous scrolling is done using a proprietary implementation and does not use
//CCParallaxNode class
-(CCParallaxNode*) paralaxNodeWithUniqueName:(NSString*)uniqueName;
-(NSArray*) spritesInParallaxNodeWithUniqueName:(NSString*)uniqueName;
//END                RETRIEVAL  METHODS
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
//                   NEW  METHODS
////////////////////////////////////////////////////////////////////////////////
//this will create a new sprite in a batch node
-(CCSprite*) newSpriteWithUniqueName:(NSString *)uniqueName;

//this will create a new sprite in the layer - no batch node
-(CCSprite*) newSpriteWithUniqueName:(NSString*)name 
                        cocos2dLayer:(CCLayer*)cocosLayer; 

//example of use
/*
 NSArray* arr = [lh newSpritesWithTag:YOUR TAG cocos2dLayer:self];
 for(CCSprite* spr in arr){...}
 */
-(NSArray*) newSpritesWithTag:(LevelHelper_TAG)tag
                        cocos2dLayer:(CCLayer*)cocosLayer;
//------------------------------------------------------------------------------

//the CCSprite associated with the body will NOT use batch nodes
-(b2Body*) newBodyWithUniqueName:(NSString*)name 
						   world:(b2World*)world 
					cocos2dLayer:(CCLayer*)cocosLayer;

//the CCSprite associated with the body will use batch nodes
-(b2Body*) newBodyWithUniqueName:(NSString*)name 
						   world:(b2World*)world;

//example of use
/*
 NSArray* arr = [lh newBodiesWithTag:YOUR TAG world:world cocos2dLayer:self];
 for(NSValue* val in arr)
 {
 b2Body* body = [val pointerValue];
 ...
 }
 */
-(NSArray*) newBodiesWithTag:(LevelHelper_TAG)tag 
							  world:(b2World*)world 
					   cocos2dLayer:(CCLayer*)cocosLayer;
////////////////////////////////////////////////////////////////////////////////
//END                NEW  METHODS
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
//                        REMOVAL  METHODS
////////////////////////////////////////////////////////////////////////////////
-(bool) removeSpriteWithUniqueName:(NSString*)name;
-(bool) removeSprite:(CCSprite*)ccsprite;
-(bool) removeAllSprites;
//------------------------------------------------------------------------------
-(bool) removeBodyWithUniqueName:(NSString*)name;
-(bool) removeBody:(b2Body*)body;
-(bool) removeBodiesWithTag:(LevelHelper_TAG)tag;
-(bool) removeAllBodies;
//------------------------------------------------------------------------------
-(bool) removeJointWithUniqueName:(NSString*)name;
//use this method to remove multiple joints at once
-(void) removeAllJointsWithTag:(LevelHelper_TAG)tag;
-(bool) removeJoint:(b2Joint*) joint;
-(bool) removeAllJoints;
////////////////////////////////////////////////////////////////////////////////
//END                     REMOVAL  METHODS
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
//                        HELPER  METHODS
////////////////////////////////////////////////////////////////////////////////
-(unsigned int) numberOfBatchNodesUsed;
-(bool) isSpriteWithUniqueName:(NSString*)name atPoint:(CGPoint)point;
////////////////////////////////////////////////////////////////////////////////
//END                     HELPER  METHODS
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
//                         GRAVITY METHODS
////////////////////////////////////////////////////////////////////////////////
-(bool) isGravityZero;
-(void) createGravity:(b2World*)world;
//END                      GRAVITY METHODS
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
//                    WORLD BOUNDARIES  METHODS
////////////////////////////////////////////////////////////////////////////////
-(bool) hasWorldBoundaries;
-(b2Body*) createWorldBoundaries:(b2World*)world;
-(b2Body*) worldBoundariesBody; //provided for simplicity - you can take this body from createWorldBoundaries method also
-(bool) removeWorldBoundaries;
//END                  WORLD BOUNDARIES  METHODS
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
//                          PATH METHODS
////////////////////////////////////////////////////////////////////////////////
-(void) moveSpriteWithUniqueName:(NSString*)sprName
            onPathWithUniqueName:(NSString*)uniqueName 
                           speed:(float)pathSpeed 
                      startPoint:(int)startPoint; //0 first point - 1 end Point

-(void) moveSprite:(CCSprite *)ccsprite 
onPathWithUniqueName:(NSString*)uniqueName 
             speed:(float)pathSpeed 
        startPoint:(int)startPoint; //0 first point - 1 end Point

//DISCUSSION
//signature for registered method should be like this: -(void)spriteMoveOnPathEnded:(CCSprite*)spr
//registration is done like this: [loader registerNotifierOnPathEndPoints:self selector:@selector(spriteMoveOnPathEnded:)];
-(void) registerNotifierOnPathEndPoints:(id)obj selector:(SEL)sel;
//END                        PATH METHODS
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
//                         ANIMATION METHODS
////////////////////////////////////////////////////////////////////////////////
//DISCUSSION
/*
 It is very IMPORTANT that an animation started on a sprite to have the same texture as the sprite (use the same image file)
 */
-(void) startAnimationWithUniqueName:(NSString *)animName onSprite:(CCSprite*)ccsprite;
-(void) startAnimationWithUniqueName:(NSString*)animName onSpriteWithUniqueName:(NSString*)sprName;

-(void) stopAnimationWithUniqueName:(NSString*)animName onSprite:(CCSprite*)ccsprite;
-(void) stopAnimationWithUniqueName:(NSString*)animName onSpriteWithUniqueName:(NSString*)sprName;

//DISCUSSION
/*
 Sometime you may not want to run the animation - but run it frame by frame
 For example when you have a block in multiple states (not braked, half braked, full braked) and you want to change the state when the block is hit
 This two methods work only if the sprite with the assigned animation is in scene.
 */
-(void) nextFrameForSprite:(CCSprite*)ccsprite;
//will reset to first frame when it reaches the end
-(void) nextFrameForSprite:(CCSprite*)ccsprite repeat:(bool)repeat;

-(void) nextFrameForSpriteWithUniqueName:(NSString*)sprName;
//will reset to first frame when it reaches the end
-(void) nextFrameForSpriteWithUniqueName:(NSString*)sprName repeat:(bool)repeat;

-(void) prevFrameForSprite:(CCSprite*)spr;
//will reset to last frame when it reaches the begining
-(void) prevFrameForSprite:(CCSprite*)spr repeat:(bool)repeat;

-(void) prevFrameForSpriteWithUniqueName:(NSString*)sprName;
//will reset to last frame when it reaches the begining
-(void) prevFrameForSpriteWithUniqueName:(NSString*)sprName repeat:(bool)repeat;

//DISCUSSION
//signature for registered method should be like this: -(void) spriteAnimHasEnded:(CCSprite*)spr animationName:(NSString*)animName
//registration is done like this: [loader registerNotifierOnAnimationEnds:self selector:@selector(spriteAnimHasEnded:animationName:)];
-(void) registerNotifierOnAnimationEnds:(id)obj selector:(SEL)sel;

//DISCUSSION
/*
 by default the notification on animation end works only on non-"loop forever" animations
 if you want to receive notifications on "loop forever" animations enable this behaviour
 before addObjectsToWorld by calling the following function
 */
-(void) enableNotifOnLoopForeverAnimations;
//END                         ANIMATION METHODS
////////////////////////////////////////////////////////////////////////////////


//needed when deriving this class
-(void) setSpriteProperties:(CCSprite*)ccsprite spriteProperties:(NSDictionary*)spriteProp;
////////////////////////////////////////////////////////////////////////////////
@end





