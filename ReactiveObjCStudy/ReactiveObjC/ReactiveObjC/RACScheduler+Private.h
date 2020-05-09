//
//  RACScheduler+Private.h
//  ReactiveObjC
//
//  Created by Josh Abernathy on 11/29/12.
//  Copyright (c) 2012 GitHub, Inc. All rights reserved.
//

#import "RACScheduler.h"

NS_ASSUME_NONNULL_BEGIN

// The thread-specific current scheduler key.
extern NSString * const RACSchedulerCurrentSchedulerKey;

// A private interface for internal RAC use only.
@interface RACScheduler ()

 /*
  A dedicated scheduler that fills two requirements:
  
  1. By the time subscription happens, we need a valid +currentScheduler.
  2. Subscription should happen as soon as possible.
  
  To fulfill those two, if we already have a valid +currentScheduler, it immediately executes scheduled blocks. If we don't, it will execute scheduled blocks with a private background scheduler.
  
  一个专用的调度，满足两个要求:
  
  1. 在订阅发生时，我们需要一个有效的+currentScheduler。
  2. 订阅应该尽快发生。
  
  要实现这两点，如果我们已经有一个有效的+currentScheduler，它将立即执行调度块。如果没有，它将使用私有后台调度执行计划的块。
 */
+ (instancetype)subscriptionScheduler;

// Initializes the receiver with the given name.
//
// name - The name of the scheduler. If nil, a default name will be used.
//
// Returns the initialized object.
- (instancetype)initWithName:(nullable NSString *)name;

@end

NS_ASSUME_NONNULL_END
