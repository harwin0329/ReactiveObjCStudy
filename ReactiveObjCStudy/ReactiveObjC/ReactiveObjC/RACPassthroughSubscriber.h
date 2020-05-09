//
//  RACPassthroughSubscriber.h
//  ReactiveObjC
//
//  Created by Justin Spahr-Summers on 2013-06-13.
//  Copyright (c) 2013 GitHub, Inc. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "RACSubscriber.h"

@class RACCompoundDisposable;
@class RACSignal<__covariant ValueType>;

/*
 A private subscriber that passes through all events to another subscriber while not disposed.
 一个私有subscriber，在未释放时将所有事件传递给另一个subscriber。
*/
@interface RACPassthroughSubscriber : NSObject <RACSubscriber>

/*
 Initializes the receiver to pass through events until disposed.

 subscriber - The subscriber to forward events to. This must not be nil.
 signal     - The signal that will be sending events to the receiver.
 disposable - When this disposable is disposed, no more events will be forwarded. This must not be nil.

 Returns an initialized passthrough subscriber.

 初始化接收方，使其通过事件直到释放。

 订阅者——将事件转发给的订阅者。这个不能是nil。
 信号——将向接收者发送事件的信号。
 一次性-当这个一次性的处理，没有更多的事件将被转发。这个不能是nil。

 返回一个初始化的passthroughSubscriber。
*/
- (instancetype)initWithSubscriber:(id<RACSubscriber>)subscriber signal:(RACSignal *)signal disposable:(RACCompoundDisposable *)disposable;

@end
