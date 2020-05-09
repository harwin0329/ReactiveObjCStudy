//
//  RACDynamicSignal.m
//  ReactiveObjC
//
//  Created by Justin Spahr-Summers on 2013-10-10.
//  Copyright (c) 2013 GitHub, Inc. All rights reserved.
//

#import "RACDynamicSignal.h"
#import "RACEXTScope.h"
#import "RACCompoundDisposable.h"
#import "RACPassthroughSubscriber.h"
#import "RACScheduler+Private.h"
#import "RACSubscriber.h"
#import <libkern/OSAtomic.h>

@interface RACDynamicSignal ()

// The block to invoke for each subscriber.
@property (nonatomic, copy, readonly) RACDisposable * (^ didSubscribe)(id<RACSubscriber> subscriber);

@end

@implementation RACDynamicSignal

#pragma mark Lifecycle

+ (RACSignal *)createSignal:(RACDisposable * (^)(id<RACSubscriber> subscriber))didSubscribe {
    RACDynamicSignal *signal = [[self alloc] init];
    signal->_didSubscribe = [didSubscribe copy];//block copy
    return [signal setNameWithFormat:@"+createSignal:"];
    /*
     1.创建 signal 信号对象的时候，最终创建的是 RACSignal 的子类 RACDynamicSignal 的对象；
     2.传入一个名为 didSubscribe，参数为 subscriber 订阅者对象，返回值为 RACDisposable 对象的 block回调，保存到 signal 对象中；
     3.此时信号为冷信号
     */
}

#pragma mark Managing Subscribers

- (RACDisposable *)subscribe:(id<RACSubscriber>)subscriber {
    NSCParameterAssert(subscriber != nil);

    RACCompoundDisposable *disposable = [RACCompoundDisposable compoundDisposable];
    /*
     1.RACCompoundDisposable：RACDisposable的子类，可以加入多个RACDisposable对象。
     2.当RACCompoundDisposable对象被dispose的时候，会dispose容器内的所有RACDisposable对象。
    */
    subscriber = [[RACPassthroughSubscriber alloc] initWithSubscriber:subscriber signal:self disposable:disposable];
    /*
     1.将subscriber 用 RACPassthroughSubcriber 创建的对象来替代;
     2.RACPassthroughSubscriber类中保存了3个非常重要的对象，RACSubscriber，RACSignal，RACCompoundDisposable；
     3.RACSubscriber是待转发的信号的订阅者subscriber；
     4.RACCompoundDisposable是订阅者的销毁对象，一旦它被disposed了，innerSubscriber就再也接受不到事件流了。
     5.通过RACPassthroughSubscriber对象来转发给真正的Subscriber
    */

    if (self.didSubscribe != NULL) {
        // self.didSubscribe 即 创建信号时传入的signal->_didSubscribe = [didSubscribe copy];
        RACDisposable *schedulingDisposable = [RACScheduler.subscriptionScheduler schedule:^{
            RACDisposable *innerDisposable = self.didSubscribe(subscriber);
            /*
             1.执行disSubscribe block
             2.进入didSubscribe闭包后，调用sendNext:、sendError:、sendCompleted。
             3.由于已经将subscriber替换为RACPassthroughSubscriber对象，真正的subscriber被存储在RACPassthroughSubscriber对象中，即innerSubscriber，所以这一步的各种send方法其实是一个转发过程。
             
            */
            
            [disposable addDisposable:innerDisposable];
        }];

        [disposable addDisposable:schedulingDisposable];
        /*
         1.在取currentScheduler的过程中，会判断currentScheduler是否存在，和是否在主线程中。如果都没有，那么就会调用后台backgroundScheduler去执行schedule的block。
         2.self.didSubscribe(subscriber)这一句就执行了信号保存的didSubscribe闭包，就是之前在信号里发送信息，sendNext、sendError、sendComplete。
         3.此时的订阅者是RACPassthroughSubcriber,但是最终的订阅者是innerSubscriber。
        */
    }

    return disposable;
    /*
     1.订阅，参数为 subscriber 对象，返回值为 RACDisposable 对象，此方法中会调用 signal 对象中保存的 didSubscribe 回调；
     2.信号变为热信号
    */
}

@end
