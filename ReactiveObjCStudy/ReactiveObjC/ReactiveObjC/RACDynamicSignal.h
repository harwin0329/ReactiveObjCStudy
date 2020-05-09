//
//  RACDynamicSignal.h
//  ReactiveObjC
//
//  Created by Justin Spahr-Summers on 2013-10-10.
//  Copyright (c) 2013 GitHub, Inc. All rights reserved.
//

#import "RACSignal.h"

 /*
  A private `RACSignal` subclasses that implements its subscription behavior using a block.
  
  一个私有的' RACSignal '子类，它使用一个块实现其订阅行为。
  
  dynamic 动态
 */
@interface RACDynamicSignal : RACSignal

+ (RACSignal *)createSignal:(RACDisposable * (^)(id<RACSubscriber> subscriber))didSubscribe;

@end
