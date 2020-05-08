//
//  ViewController.m
//  ReactiveObjCStudy
//
//  Created by Harwin on 2020/5/7.
//  Copyright © 2020 WoQi. All rights reserved.
//

#import "ViewController.h"
#import "Person.h"
#import "ReactiveObjC.h"

@interface ViewController ()

@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view.
    
    RACStream;
    
    [self testValueType];
}

// 测试泛型
- (void)testValueType {
    Language *language = [[Java alloc]init];
    Java *java = [[Java alloc]init];
    iOS *ios = [[iOS alloc]init];

    Person<Language *> *p = [[Person alloc] init];
    Person<Java *> *p2 = [[Person alloc] init];
    Person<iOS *> *p3 = [[Person alloc] init];

    p.language = language;
    p2.language = java;
    p3.language = ios;

    /**
     泛型
     */

    /**
     泛型 __covariant 子类转父类
     */
    p = p2;

    /**
     泛型 __contravariant 可以逆变,父类转子类
     */
    p3 = p;
}

@end
