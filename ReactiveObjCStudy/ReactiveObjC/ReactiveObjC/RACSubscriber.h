//
//  RACSubscriber.h
//  ReactiveObjC
//
//  Created by Josh Abernathy on 3/1/12.
//  Copyright (c) 2012 GitHub, Inc. All rights reserved.
//

#import <Foundation/Foundation.h>

@class RACCompoundDisposable;

NS_ASSUME_NONNULL_BEGIN

 /*
  Represents any object which can directly receive values from a RACSignal.

  You generally shouldn't need to implement this protocol. +[RACSignal createSignal:], RACSignal's subscription methods, or RACSubject should work for most uses.

  Implementors of this protocol may receive messages and values from multiple threads simultaneously, and so should be thread-safe. Subscribers will also be weakly referenced so implementations must allow that.
  
  表示可以直接从RACSignal接收值的任何对象。
  
  通常不需要实现这个协议。+[RACSignal createSignal:]， RACSignal的订阅方法，或RACSubject应该在大多数情况下工作。
  
  此协议的实现者可能同时从多个线程接收消息和值，因此应该是线程安全的。订阅者也将被弱引用，因此实现必须允许这样做。
 */
@protocol RACSubscriber <NSObject>
@required

// Sends the next value to subscribers.
//
// value - The value to send. This can be `nil`.
- (void)sendNext:(nullable id)value;

// Sends the error to subscribers.
//
// error - The error to send. This can be `nil`.
//
// This terminates the subscription, and invalidates the subscriber (such that
// it cannot subscribe to anything else in the future).
- (void)sendError:(nullable NSError *)error;

// Sends completed to subscribers.
//
// This terminates the subscription, and invalidates the subscriber (such that
// it cannot subscribe to anything else in the future).
- (void)sendCompleted;

// Sends the subscriber a disposable that represents one of its subscriptions.
//
// A subscriber may receive multiple disposables if it gets subscribed to
// multiple signals; however, any error or completed events must terminate _all_
// subscriptions.
- (void)didSubscribeWithDisposable:(RACCompoundDisposable *)disposable;

@end

NS_ASSUME_NONNULL_END
