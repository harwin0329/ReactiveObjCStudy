//
//  RACSubscriptionScheduler.h
//  ReactiveObjC
//
//  Created by Josh Abernathy on 11/30/12.
//  Copyright (c) 2012 GitHub, Inc. All rights reserved.
//

#import "RACScheduler.h"

NS_ASSUME_NONNULL_BEGIN

 /*
  private scheduler used only for subscriptions. See the private +[RACScheduler subscriptionScheduler] method for more information.
  
  仅用于订阅的私有调度。有关更多信息，请参见 +[RACScheduler subscriptionScheduler] 私有方法。
 */
@interface RACSubscriptionScheduler : RACScheduler

@end

NS_ASSUME_NONNULL_END
