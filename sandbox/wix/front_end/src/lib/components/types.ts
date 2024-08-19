export type Range = {
    id: number;
    value: number;
    min: number;
    max: number;
    step:number;
    label: string;
};
export type Widget = {
    range?: Range;
};