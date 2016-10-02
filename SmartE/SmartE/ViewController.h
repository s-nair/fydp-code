//
//  ViewController.h
//  SmartE
//
//  Created by Gary Singh on 2016-07-17.
//  Copyright © 2016 GarySinghMann. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface ViewController : UIViewController<NSStreamDelegate>

@property (nonatomic, retain) NSInputStream *inputStream;
@property (nonatomic, retain) NSOutputStream *outputStream;

@property (weak, nonatomic) IBOutlet UISegmentedControl *lightToggle;
- (IBAction)toggleLight:(id)sender;

@property (weak, nonatomic) IBOutlet UIButton *retrieveLightStatusButton;
@property (weak, nonatomic) IBOutlet UILabel *lightStatus;
- (IBAction)retrieveLightStatus:(id)sender;


@end

